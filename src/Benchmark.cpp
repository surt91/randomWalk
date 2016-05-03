#include "Benchmark.hpp"

std::unique_ptr<Walker> run_walker(const Cmd &o)
{
    clock_t before_walker = clock();
    std::unique_ptr<Walker> w;
    UniformRNG rng(o.seedRealization);
    if(o.type == WT_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new LatticeWalker(o.d, o.steps, rng, o.chAlg));
    else if(o.type == WT_LOOP_ERASED_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, o.steps, rng, o.chAlg));
    else if(o.type == WT_SELF_AVOIDING_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new SelfAvoidingWalker(o.d, o.steps, rng, o.chAlg));
    else if(o.type == WT_REAL_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new RealWalker(o.d, o.steps, rng, o.chAlg));
    else if(o.type == WT_GAUSSIAN_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new GaussWalker(o.d, o.steps, rng, o.chAlg));
    else
        LOG(LOG_ERROR) << "cannot find walk type " << o.type;

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

    if(std::abs(w->L() - o.benchmark_L) > 1e-4)
    {
        LOG(LOG_ERROR) << "Wrong L  " << w->L();
        LOG(LOG_ERROR) << "expected " << o.benchmark_L;
    }
    if(std::abs(w->A() - o.benchmark_A) > 1e-4)
    {
        LOG(LOG_ERROR) << "Wrong A  " << w->A();
        LOG(LOG_ERROR) << "expected " << o.benchmark_A;
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
    const double threshold = 1e-4;
    // 1e3 iterations, should be 10% accurate (?)
    if(std::abs(s.sum_L / o.iterations - expected_mean_L) > threshold)
    {
        LOG(LOG_ERROR) << "Wrong <L>  " << s.sum_L / o.iterations;
        LOG(LOG_ERROR) << "expected " << expected_mean_L;
    }
    if(std::abs(s.sum_A / o.iterations - expected_mean_A) > threshold)
    {
        LOG(LOG_ERROR) << "Wrong <A>  " << s.sum_A / o.iterations;
        LOG(LOG_ERROR) << "expected " << expected_mean_A;
    }
    if(std::abs(s.sum_r / o.iterations - expected_mean_r) > threshold)
    {
        LOG(LOG_ERROR) << "Wrong <r>  " << s.sum_r / o.iterations;
        LOG(LOG_ERROR) << "expected " << expected_mean_r;
    }
    if(std::abs(s.sum_r2 / o.iterations - expected_mean_r2) > threshold)
    {
        LOG(LOG_ERROR) << "Wrong <r2>  " << s.sum_r2 / o.iterations;
        LOG(LOG_ERROR) << "expected " << expected_mean_r2;
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
    o.d = 2;
    o.wantedObservable = WO_VOLUME;

    o.simpleSampling = true;

    clock_t start = clock();
    LOG(LOG_INFO) << "Starting Simulations";

    o.data_path = "bench.tmp";
    o.chAlg = CH_ANDREWS_AKL;
    for(int i=1; i<=5; ++i)
    {
        o.t_eq = -1;
        o.t_eqMax = 1000;
        o.simpleSampling = true;
        double expected_mean_L;
        double expected_mean_A;
        double expected_mean_r;
        double expected_mean_r2;
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
                expected_mean_L = 85.33;
                expected_mean_A = 349.06;
                expected_mean_r = 28.74;
                expected_mean_r2 = 845.95;
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
                expected_mean_L = 58.86;
                expected_mean_A = 187.81;
                expected_mean_r = 15.67;
                expected_mean_r2 = 281.49;
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

    for(int i=1; i<=5; ++i)
    {
        switch(i)
        {
            case WT_RANDOM_WALK:
                o.steps = 1000000;
                o.type = WT_RANDOM_WALK;
                o.benchmark_L = 3747.1842096;
                o.benchmark_A = 984338;
                o.iterations = 10;
                break;
            case WT_LOOP_ERASED_RANDOM_WALK:
                o.steps = 10000;
                o.type = WT_LOOP_ERASED_RANDOM_WALK;
                o.benchmark_L = 4063.70552402;
                o.benchmark_A = 481513.5;
                o.iterations = 10;
                break;
            case WT_SELF_AVOIDING_RANDOM_WALK:
                o.steps = 320;
                o.type = WT_SELF_AVOIDING_RANDOM_WALK;
                o.benchmark_L = 153.467524349;
                o.benchmark_A = 1614.5;
                o.iterations = 10000;
                break;
            case WT_REAL_RANDOM_WALK:
                o.steps = 1000000;
                o.type = WT_REAL_RANDOM_WALK;
                o.benchmark_L = 3301.2873826;
                o.benchmark_A = 696563.540913;
                o.iterations = 10;
                break;
            case WT_GAUSSIAN_RANDOM_WALK:
                o.steps = 1000000;
                o.type = WT_GAUSSIAN_RANDOM_WALK;
                o.benchmark_L = 4276.37295704;
                o.benchmark_A = 1114653.50651;
                o.iterations = 10;
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
    }

    LOG(LOG_TIMING) << "Total : " << time_diff(mid, clock());
    LOG(LOG_TIMING) << "Mem: " << vmPeak();

    LOG(LOG_TIMING) << "Grand Total : " << time_diff(start, clock());
}
