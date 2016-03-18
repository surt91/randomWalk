#include "Benchmark.hpp"

std::string time_diff(clock_t start, clock_t end)
{
    return std::to_string((double)(end - start) / CLOCKS_PER_SEC) + "s";
}

void run_walker_and_CH(Cmd o)
{
    std::vector<double> numbers = rng(o.steps, o.seedRealization);

    clock_t before_walker = clock();
    std::unique_ptr<Walker> w;
    if(o.type == 1)
        w = std::unique_ptr<Walker>(new Walker(o.d, numbers, o.chAlg));
    else if(o.type == 2)
        w = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, numbers, o.chAlg));
    w->steps();

    clock_t before_ch = clock();
    w->convexHull();

    clock_t before_output = clock();

    if(std::abs(w->L() - o.benchmark_L) > 0.01)
    {
        log<LOG_ERROR>("Wrong L ") << w->L();
        log<LOG_ERROR>("expected") << o.benchmark_L;
    }
    if(std::abs(w->A() - o.benchmark_A) > 1)
    {
        log<LOG_ERROR>("Wrong A ") << w->A();
        log<LOG_ERROR>("expected") << o.benchmark_A;
    }

    log<LOG_TIMING>("RW : ") << time_diff(before_ch, before_walker);
    log<LOG_TIMING>("CH : ") << time_diff(before_output, before_ch);
}

void run_MC_simulation(Cmd o)
{
}

void benchmark()
{
    Logger::verbosity = LOG_TIMING;

    Cmd o;
    o.benchmark = true;
    o.steps = 1000000;
    o.seedRealization = 13;
    o.seedMC = 42;
    o.d = 2;


    o.type = 1;
    o.benchmark_L = 3747.18;
    o.benchmark_A = 984338;

    log<LOG_INFO>("Random Walk, Andrews Monotone Chain");
    o.chAlg = CH_ANDREWS;
    run_walker_and_CH(o);

    log<LOG_INFO>("Random Walk, Andrews Monotone Chain and Akl Toussaint");
    o.chAlg = CH_ANDREWS_AKL;
    run_walker_and_CH(o);

    log<LOG_INFO>("Random Walk, QHull");
    o.chAlg = CH_QHULL;
    run_walker_and_CH(o);

    log<LOG_INFO>("Random Walk, QHull and Akl Toussaint");
    o.chAlg = CH_QHULL_AKL;
    run_walker_and_CH(o);


    o.type = 2;
    o.benchmark_L = 3097.93;
    o.benchmark_A = 581186;

    log<LOG_INFO>("Loop Erased Random Walk, Andrews Monotone Chain");
    o.chAlg = CH_ANDREWS;
    run_walker_and_CH(o);

    log<LOG_INFO>("Loop Erased Random Walk, Andrews Monotone Chain and Akl Toussaint");
    o.chAlg = CH_ANDREWS_AKL;
    run_walker_and_CH(o);

    log<LOG_INFO>("Loop Erased Random Walk, QHull");
    o.chAlg = CH_QHULL;
    run_walker_and_CH(o);

    log<LOG_INFO>("Loop Erased Random Walk, QHull and Akl Toussaint");
    o.chAlg = CH_QHULL_AKL;
    run_walker_and_CH(o);

    //~ o.iterations = 1;
    //~ o.theta = 1;
}
