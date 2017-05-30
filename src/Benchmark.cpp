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

bool run_hull(const Cmd &o, std::unique_ptr<Walker> &w)
{
    bool fail = false;
    clock_t before_ch = clock();
    w->setHullAlgo(o.chAlg);

    clock_t before_output = clock();

    if(std::abs((w->L() - o.benchmark_L)/o.benchmark_L) > 1e-6)
    {
        LOG(LOG_ERROR) << "Wrong L  " << w->L();
        LOG(LOG_ERROR) << "expected " << o.benchmark_L;
        fail = true;
    }
    if(std::abs((w->A() - o.benchmark_A)/o.benchmark_A) > 1e-6)
    {
        LOG(LOG_ERROR) << "Wrong A  " << w->A();
        LOG(LOG_ERROR) << "expected " << o.benchmark_A;
        fail = true;
    }

    LOG(LOG_TIMING) << "            " << time_diff(before_ch, before_output);

    return fail;
}

bool run_simulation(const Cmd &o, double expected_mean_A,
                                  double expected_mean_L,
                                  double expected_mean_r,
                                  double expected_mean_r2)
{
    bool fail = false;
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
        fail = true;
    }
    if(std::abs(s.sum_A / o.iterations - expected_mean_A) > threshold)
    {
        LOG(LOG_ERROR) << "Wrong <A>  " << s.sum_A / o.iterations;
        LOG(LOG_ERROR) << "expected " << expected_mean_A;
        fail = true;
    }
    if(std::abs(s.sum_r / o.iterations - expected_mean_r) > threshold)
    {
        LOG(LOG_ERROR) << "Wrong <r>  " << s.sum_r / o.iterations;
        LOG(LOG_ERROR) << "expected " << expected_mean_r;
        fail = true;
    }
    if(std::abs(s.sum_r2 / o.iterations - expected_mean_r2) > threshold)
    {
        LOG(LOG_ERROR) << "Wrong <r2>  " << s.sum_r2 / o.iterations;
        LOG(LOG_ERROR) << "expected " << expected_mean_r2;
        fail = true;
    }

    LOG(LOG_TIMING) << "    " << time_diff(before_mc, clock());

    return fail;
}

bool benchmark()
{
    bool fail = false;
    Logger::verbosity = LOG_TIMING;

    Cmd o;
    o.benchmark = true;
    o.seedRealization = 13;
    o.seedMC = 42;
    o.wantedObservable = WO_VOLUME;
    o.numWalker = 1;

    o.t_eq = -1;
    o.t_eqMax = 1000;
    o.simpleSampling = true;
    o.sampling_method = SM_METROPOLIS;

    clock_t start = clock();
    LOG(LOG_INFO) << "Starting 2D Simple Sampling Simulations";

    o.data_path = "bench.tmp";

    o.d = 2;
    o.chAlg = CH_ANDREWS_AKL;
    o.iterations = 1000;
    for(int i=1; i<=8; ++i)
    {
        double expected_mean_L = 0;
        double expected_mean_A = 0;
        double expected_mean_r = 0;
        double expected_mean_r2 = 0;

        switch(i)
        {
            case WT_RANDOM_WALK:
                o.steps = 150;
                o.sweep = o.steps;
                o.type = WT_RANDOM_WALK;
                expected_mean_L = 48.38;
                expected_mean_A = 120.22;
                expected_mean_r = 12.80;
                expected_mean_r2 = 177.25;
                break;
            case WT_LOOP_ERASED_RANDOM_WALK:
                o.steps = 30;
                o.sweep = o.steps;
                o.type = WT_LOOP_ERASED_RANDOM_WALK;
                expected_mean_L = 35.1526753033;
                expected_mean_A = 56.32;
                expected_mean_r = 12.519018525;
                expected_mean_r2 = 159.618;
                break;
            case WT_SELF_AVOIDING_RANDOM_WALK:
                o.steps = 100;
                o.sweep = o.steps;
                o.type = WT_SELF_AVOIDING_RANDOM_WALK;
                expected_mean_L = 84.7176752612;
                expected_mean_A = 344.4645;
                expected_mean_r = 28.782060404;
                expected_mean_r2 = 849.638;
                break;
            case WT_REAL_RANDOM_WALK:
                o.steps = 130;
                o.sweep = o.steps;
                o.type = WT_REAL_RANDOM_WALK;
                expected_mean_L = 43.10;
                expected_mean_A = 100.23;
                expected_mean_r = 11.58;
                expected_mean_r2 = 154.77;
                break;
            case WT_GAUSSIAN_RANDOM_WALK:
                o.steps = 130;
                o.sweep = o.steps;
                o.type = WT_GAUSSIAN_RANDOM_WALK;
                expected_mean_L = 59.1868262964;
                expected_mean_A = 189.601620782;
                expected_mean_r = 15.8136986085;
                expected_mean_r2 = 285.623373981;
                break;
            case WT_LEVY_FLIGHT:
                o.steps = 130;
                o.sweep = o.steps;
                o.type = WT_LEVY_FLIGHT;
                expected_mean_L = 1721.50;
                expected_mean_A = 96667.18;
                expected_mean_r = 759.03;
                expected_mean_r2 = 7875699.19;
                break;
            case WT_CORRELATED_RANDOM_WALK:
                o.steps = 80;
                o.sweep = o.steps;
                o.type = WT_CORRELATED_RANDOM_WALK;
                expected_mean_L = 31.1292517867;
                expected_mean_A = 49.253667193;
                expected_mean_r = 9.17497080667;
                expected_mean_r2 = 96.0109448785;
                break;
            case WT_ESCAPE_RANDOM_WALK:
                o.steps = 30;
                o.sweep = o.steps;
                o.type = WT_ESCAPE_RANDOM_WALK;
                expected_mean_L = 29.0173126288;
                expected_mean_A = 42.0055;
                expected_mean_r = 9.04605822968;
                expected_mean_r2 = 88.092;
                break;
        }
        LOG(LOG_INFO) << TYPE_LABEL[i];
        fail |= run_simulation(o,
                        expected_mean_A,
                        expected_mean_L,
                        expected_mean_r,
                        expected_mean_r2);
    }

    clock_t mid1 = clock();
    LOG(LOG_TIMING) << "Total : " << time_diff(start, mid1);
    LOG(LOG_TIMING) << "Mem: " << vmPeak();

    LOG(LOG_INFO) << "Starting 3D Importance Sampling Simulations";
    o.t_eq = 0;
    o.simpleSampling = false;
    o.theta = -20;
    o.d = 3;
    o.chAlg = CH_QHULL;
    for(int i=1; i<=8; ++i)
    {
        double expected_mean_L = 0;
        double expected_mean_A = 0;
        double expected_mean_r = 0;
        double expected_mean_r2 = 0;

        switch(i)
        {
            case WT_RANDOM_WALK:
                o.steps = 150;
                o.sweep = o.steps;
                o.type = WT_RANDOM_WALK;
                o.iterations = 100;
                expected_mean_L = 4869.41598187;
                expected_mean_A = 19565.55;
                expected_mean_r = 69.9165860861;
                expected_mean_r2 = 4928.82;
                break;
            case WT_LOOP_ERASED_RANDOM_WALK:
                o.steps = 30;
                o.sweep = o.steps;
                o.type = WT_LOOP_ERASED_RANDOM_WALK;
                o.iterations = 600;
                expected_mean_L = 87.0967246288;
                expected_mean_A = 47.6158333333;
                expected_mean_r = 8.34690569726;
                expected_mean_r2 = 76.3566666667;
                break;
            case WT_SELF_AVOIDING_RANDOM_WALK:
                o.steps = 120;
                o.sweep = o.steps;
                o.type = WT_SELF_AVOIDING_RANDOM_WALK;
                o.iterations = 100;
                expected_mean_L = 3193.39165733;
                expected_mean_A = 9521.41833333;
                expected_mean_r = 59.7190330182;
                expected_mean_r2 = 3631.4;
                break;
            case WT_REAL_RANDOM_WALK:
                o.steps = 100;
                o.sweep = o.steps;
                o.type = WT_REAL_RANDOM_WALK;
                o.iterations = 100;
                expected_mean_L = 2264.79537161;
                expected_mean_A = 6891.25326374;
                expected_mean_r = 50.2614559199;
                expected_mean_r2 = 2578.81224846;
                break;
            case WT_GAUSSIAN_RANDOM_WALK:
                o.steps = 100;
                o.sweep = o.steps;
                o.type = WT_GAUSSIAN_RANDOM_WALK;
                o.iterations = 100;
                expected_mean_L = 15396.0001625;
                expected_mean_A = 124060.451034;
                expected_mean_r = 123.258862156;
                expected_mean_r2 = 15607.6366561;
                break;
            case WT_LEVY_FLIGHT:
                o.steps = 130;
                o.sweep = o.steps;
                o.type = WT_LEVY_FLIGHT;
                o.iterations = 100;
                expected_mean_L = 73197525.4829;
                expected_mean_A = 2817982513.32;
                expected_mean_r = 768.920252627;
                expected_mean_r2 = 5540570.55171;
                break;
            case WT_CORRELATED_RANDOM_WALK:
                o.steps = 90;
                o.sweep = o.steps;
                o.type = WT_CORRELATED_RANDOM_WALK;
                expected_mean_L = 744.279070151;
                expected_mean_A = 1323.8847083;
                expected_mean_r = 25.8766506636;
                expected_mean_r2 = 710.389952596;
                break;
            case WT_ESCAPE_RANDOM_WALK:
                o.steps = 40;
                o.sweep = o.steps;
                o.type = WT_ESCAPE_RANDOM_WALK;
                o.iterations = 100;
                expected_mean_L = 118.04597545;
                expected_mean_A = 80.2966666667;
                expected_mean_r = 8.81999414734;
                expected_mean_r2 = 87.28;
                break;
        }
        LOG(LOG_INFO) << TYPE_LABEL[i];
        fail |= run_simulation(o,
                        expected_mean_A,
                        expected_mean_L,
                        expected_mean_r,
                        expected_mean_r2);
    }

    clock_t mid2 = clock();
    LOG(LOG_TIMING) << "Total : " << time_diff(mid1, mid2);
    LOG(LOG_TIMING) << "Mem: " << vmPeak();

    LOG(LOG_INFO) << "Starting Hull Comparison";

    for(int i=1; i<=8; ++i)
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
                o.benchmark_L = 2989.94168838;
                o.benchmark_A = 538864.5;
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
            case WT_ESCAPE_RANDOM_WALK:
                o.steps = 10000;
                o.type = WT_ESCAPE_RANDOM_WALK;
                o.benchmark_L = 963.31167262;
                o.benchmark_A = 60810.5;
                break;
        }

        LOG(LOG_INFO) << TYPE_LABEL[i];
        auto w = run_walker(o);

        for(int j=1; j<=4; ++j)
        {
            o.chAlg = (hull_algorithm_t) j;
            LOG(LOG_INFO) << "        " << CH_LABEL[j];
            try{
                fail |= run_hull(o, w);
            } catch(...) {
                fail = true;
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
                o.benchmark_L = 156915.992382;
                o.benchmark_A = 3929040.66667;
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
            case WT_ESCAPE_RANDOM_WALK:
                o.steps = 10000;
                o.type = WT_ESCAPE_RANDOM_WALK;
                o.benchmark_L = 25552.1280682;
                o.benchmark_A = 319318.833333;
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
                fail |= run_hull(o, w);
            } catch(...) {
                fail = true;
                continue;
            }
        }
    }

    LOG(LOG_TIMING) << "Total : " << time_diff(mid2, clock());
    LOG(LOG_TIMING) << "Mem: " << vmPeak();

    LOG(LOG_TIMING) << "Grand Total : " << time_diff(start, clock());

    if(fail)
    {
        LOG(LOG_ERROR) << "Some tests failed!";
    }

    return fail;
}
