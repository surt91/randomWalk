#include "Benchmark.hpp"

std::unique_ptr<Walker> run_walker(const Cmd &o)
{
    clock_t before_walker = clock();
    std::unique_ptr<Walker> w;
    UniformRNG rng(o.seedRealization);
    Simulation::prepare(w, o);

    clock_t before_points = clock();
    LOG(LOG_TIMING) << "    " << time_diff(before_walker, before_points);

    w->updatePoints();

    LOG(LOG_TIMING) << "    " << time_diff(before_points, clock());

    return w;
}

void run_hull(const Cmd &o, std::unique_ptr<Walker> &w)
{
    clock_t before_ch = clock();
    w->setHullAlgo(o.chAlg);

    clock_t before_output = clock();

    if(std::abs((w->L() - o.benchmark_L)/o.benchmark_L) > 1e-6)
    {
        LOG(LOG_ERROR) << "Wrong L  " << w->L();
        LOG(LOG_ERROR) << "expected " << o.benchmark_L;
        exit(1);
    }
    if(std::abs((w->A() - o.benchmark_A)/o.benchmark_A) > 1e-6)
    {
        LOG(LOG_ERROR) << "Wrong A  " << w->A();
        LOG(LOG_ERROR) << "expected " << o.benchmark_A;
        exit(1);
    }

    LOG(LOG_TIMING) << "            " << time_diff(before_ch, before_output);
}

void run_simulation(const Cmd &o, double expected_mean_A,
                                  double expected_mean_L,
                                  double expected_mean_r,
                                  double expected_mean_r2)
{
    clock_t before_mc = clock();

    Metropolis s(o);
    s.mute();
    s.run();
    const double threshold = 1e-2;
    // 1e3 iterations, should be 10% accurate (?)
    if(std::abs(s.sum_L / o.iterations - expected_mean_L) > threshold)
    {
        LOG(LOG_ERROR) << "Wrong <L>  " << s.sum_L / o.iterations;
        LOG(LOG_ERROR) << "expected " << expected_mean_L;
        exit(1);
    }
    if(std::abs(s.sum_A / o.iterations - expected_mean_A) > threshold)
    {
        LOG(LOG_ERROR) << "Wrong <A>  " << s.sum_A / o.iterations;
        LOG(LOG_ERROR) << "expected " << expected_mean_A;
        exit(1);
    }
    if(std::abs(s.sum_r / o.iterations - expected_mean_r) > threshold)
    {
        LOG(LOG_ERROR) << "Wrong <r>  " << s.sum_r / o.iterations;
        LOG(LOG_ERROR) << "expected " << expected_mean_r;
        exit(1);
    }
    if(std::abs(s.sum_r2 / o.iterations - expected_mean_r2) > threshold)
    {
        LOG(LOG_ERROR) << "Wrong <r2>  " << s.sum_r2 / o.iterations;
        LOG(LOG_ERROR) << "expected " << expected_mean_r2;
        exit(1);
    }

    LOG(LOG_TIMING) << "    " << time_diff(before_mc, clock());
}

void benchmark()
{
    Logger::verbosity = LOG_TIMING;

    Cmd o;
    o.benchmark = true;
    o.seedRealization = 13;
    o.seedMC = 42;
    o.wantedObservable = WO_VOLUME;

    o.t_eq = -1;
    o.t_eqMax = 1000;
    o.simpleSampling = true;

    clock_t start = clock();
    LOG(LOG_INFO) << "Starting Simulations";

    o.data_path = "bench.tmp";

    o.d = 2;
    o.chAlg = CH_ANDREWS_AKL;
    for(int i=1; i<=7; ++i)
    {
        double expected_mean_L = 0;
        double expected_mean_A = 0;
        double expected_mean_r = 0;
        double expected_mean_r2 = 0;

        switch(i)
        {
            case WT_RANDOM_WALK:
                o.steps = 150;
                o.type = WT_RANDOM_WALK;
                o.iterations = 1000;
                expected_mean_L = 48.38;
                expected_mean_A = 120.22;
                expected_mean_r = 12.80;
                expected_mean_r2 = 177.25;
                break;
            case WT_LOOP_ERASED_RANDOM_WALK:
                o.steps = 30;
                o.type = WT_LOOP_ERASED_RANDOM_WALK;
                o.iterations = 1000;
                expected_mean_L = 35.36;
                expected_mean_A = 56.32;
                expected_mean_r = 12.50;
                expected_mean_r2 = 158.04;
                break;
            case WT_SELF_AVOIDING_RANDOM_WALK:
                o.steps = 100;
                o.type = WT_SELF_AVOIDING_RANDOM_WALK;
                o.iterations = 1000;
                expected_mean_L = 84.8574757548;
                expected_mean_A = 344.535;
                expected_mean_r = 28.6759;
                expected_mean_r2 = 845.092;
                break;
            case WT_REAL_RANDOM_WALK:
                o.steps = 130;
                o.type = WT_REAL_RANDOM_WALK;
                o.iterations = 1000;
                expected_mean_L = 43.10;
                expected_mean_A = 100.23;
                expected_mean_r = 11.58;
                expected_mean_r2 = 154.77;
                break;
            case WT_GAUSSIAN_RANDOM_WALK:
                o.steps = 130;
                o.type = WT_GAUSSIAN_RANDOM_WALK;
                o.iterations = 1000;
                expected_mean_L = 59.1868262964;
                expected_mean_A = 189.601620782;
                expected_mean_r = 15.8136986085;
                expected_mean_r2 = 285.623373981;
                break;
            case WT_LEVY_FLIGHT:
                o.steps = 130;
                o.type = WT_LEVY_FLIGHT;
                o.iterations = 1000;
                expected_mean_L = 1721.50;
                expected_mean_A = 96667.18;
                expected_mean_r = 759.03;
                expected_mean_r2 = 7875699.19;
                break;
            case WT_CORRELATED_RANDOM_WALK:
                o.steps = 130;
                o.type = WT_CORRELATED_RANDOM_WALK;
                o.iterations = 1000;
                expected_mean_L = 40.8028679341;
                expected_mean_A = 86.4985600763;
                expected_mean_r = 11.398686363;
                expected_mean_r2 = 147.159380854;
                break;
        }
        LOG(LOG_INFO) << TYPE_LABEL[i];
        run_simulation(o, expected_mean_A,
                          expected_mean_L,
                          expected_mean_r,
                          expected_mean_r2);
    }

    clock_t mid = clock();
    LOG(LOG_TIMING) << "Total : " << time_diff(start, mid);
    LOG(LOG_TIMING) << "Mem: " << vmPeak();

    LOG(LOG_INFO) << "Starting Hull Comparison";

    for(int i=1; i<=7; ++i)
    {
        o.d = 2;
        switch(i)
        {
            case WT_RANDOM_WALK:
                o.steps = 1000000;
                o.type = WT_RANDOM_WALK;
                o.benchmark_L = 3747.1842096;
                o.benchmark_A = 984338;
                break;
            case WT_LOOP_ERASED_RANDOM_WALK:
                o.steps = 10000;
                o.type = WT_LOOP_ERASED_RANDOM_WALK;
                o.benchmark_L = 4063.70552402;
                o.benchmark_A = 481513.5;
                break;
            case WT_SELF_AVOIDING_RANDOM_WALK:
                o.steps = 320;
                o.type = WT_SELF_AVOIDING_RANDOM_WALK;
                o.benchmark_L = 153.467524349;
                o.benchmark_A = 1614.5;
                break;
            case WT_REAL_RANDOM_WALK:
                o.steps = 1000000;
                o.type = WT_REAL_RANDOM_WALK;
                o.benchmark_L = 3301.2873826;
                o.benchmark_A = 696563.540913;
                break;
            case WT_GAUSSIAN_RANDOM_WALK:
                o.steps = 1000000;
                o.type = WT_GAUSSIAN_RANDOM_WALK;
                o.benchmark_L = 4426.65933311;
                o.benchmark_A = 1380225.20046;
                break;
            case WT_LEVY_FLIGHT:
                o.steps = 1000000;
                o.type = WT_LEVY_FLIGHT;
                o.benchmark_L = 1103099.53017;
                o.benchmark_A = 29503698362.2;
                break;
            case WT_CORRELATED_RANDOM_WALK:
                o.steps = 1000000;
                o.type = WT_CORRELATED_RANDOM_WALK;
                o.benchmark_L = 2443.76382544;
                o.benchmark_A = 403708.247784;
                break;
        }

        LOG(LOG_INFO) << TYPE_LABEL[i];
        auto w = run_walker(o);

        for(int j=1; j<=4; ++j)
        {
            o.chAlg = (hull_algorithm_t) j;
            LOG(LOG_INFO) << "        " << CH_LABEL[j];
            try{
                run_hull(o, w);
            } catch(...) {
                continue;
            }
        }

        o.d = 3;
        switch(i)
        {
            case WT_RANDOM_WALK:
                o.steps = 1000000;
                o.type = WT_RANDOM_WALK;
                o.benchmark_L = 2247256.21308;
                o.benchmark_A = 258027553;
                break;
            case WT_LOOP_ERASED_RANDOM_WALK:
                o.steps = 10000;
                o.type = WT_LOOP_ERASED_RANDOM_WALK;
                o.benchmark_L = 165485.186039;
                o.benchmark_A = 3460280;
                break;
            case WT_SELF_AVOIDING_RANDOM_WALK:
                o.steps = 320;
                o.type = WT_SELF_AVOIDING_RANDOM_WALK;
                o.benchmark_L = 1705.20312604;
                o.benchmark_A = 4176.16666667;
                break;
            case WT_REAL_RANDOM_WALK:
                o.steps = 1000000;
                o.type = WT_REAL_RANDOM_WALK;
                o.benchmark_L = 1412091.10;
                o.benchmark_A = 115804398.6;
                break;
            case WT_GAUSSIAN_RANDOM_WALK:
                o.steps = 1000000;
                o.type = WT_GAUSSIAN_RANDOM_WALK;
                o.benchmark_L = 4610931.2558;
                o.benchmark_A = 728392022.451;
                break;
            case WT_LEVY_FLIGHT:
                o.steps = 1000000;
                o.type = WT_LEVY_FLIGHT;
                o.benchmark_L = 687169048832;
                o.benchmark_A = 1.65702708575e+16;
                break;
            case WT_CORRELATED_RANDOM_WALK:
                o.steps = 1000000;
                o.type = WT_CORRELATED_RANDOM_WALK;
                o.benchmark_L = 2535998.2343;
                o.benchmark_A = 241570113.447;
                break;
        }

        LOG(LOG_INFO) << "3D " << TYPE_LABEL[i];
        o.chAlg = CH_QHULL;
        w = run_walker(o);

        for(int j=1; j<=2; ++j)
        {
            o.chAlg = (hull_algorithm_t) j;
            LOG(LOG_INFO) << "        " << CH_LABEL[j];
            try{
                run_hull(o, w);
            } catch(...) {
                continue;
            }
        }
    }

    LOG(LOG_TIMING) << "Total : " << time_diff(mid, clock());
    LOG(LOG_TIMING) << "Mem: " << vmPeak();

    LOG(LOG_TIMING) << "Grand Total : " << time_diff(start, clock());
}
