#include "Benchmark.hpp"

std::string time_diff(clock_t start, clock_t end)
{
    return std::to_string((double)(end - start) / CLOCKS_PER_SEC) + "s";
}

// taken from http://stackoverflow.com/a/478960/1698412
std::string exec(const char* cmd)
{
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
        return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}

std::string vmPeak()
{
    std::string pid = std::to_string(getpid());
    std::string cmd("grep VmPeak /proc/"+pid+"/task/"+pid+"/status");
    return exec(cmd.c_str());
}

void run_walker_and_CH(Cmd o)
{
    clock_t before_walker = clock();
    std::unique_ptr<Walker> w;
    UniformRNG rng(o.seedRealization);
    if(o.type == WT_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new Walker(o.d, o.steps, rng, o.chAlg));
    else if(o.type == WT_LOOP_ERASED_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, o.steps, rng, o.chAlg));
    w->steps();

    clock_t before_ch = clock();
    w->convexHull();

    clock_t before_output = clock();

    if(std::abs(w->L() - o.benchmark_L) > 0.01)
    {
        Logger(LOG_ERROR) << "Wrong L  " << w->L();
        Logger(LOG_ERROR) << "expected " << o.benchmark_L;
    }
    if(std::abs(w->A() - o.benchmark_A) > 1)
    {
        Logger(LOG_ERROR) <<"Wrong A  " << w->A();
        Logger(LOG_ERROR) <<"expected " << o.benchmark_A;
    }

    Logger(LOG_TIMING) << "RW : " << time_diff(before_walker, before_ch);
    Logger(LOG_TIMING) << "CH : " << time_diff(before_ch, before_output);
}

void run_MC_simulation(Cmd o)
{
}

void benchmark()
{
    Logger::verbosity = LOG_TIMING;
    Logger::verbosity = LOG_TOO_MUCH;

    Cmd o;
    o.benchmark = true;
    o.steps = 1000000;
    o.seedRealization = 13;
    o.seedMC = 42;
    o.d = 2;


    o.type = WT_RANDOM_WALK;
    o.benchmark_L = 3747.18;
    o.benchmark_A = 984338;

    Logger(LOG_INFO) << "Random Walk, Andrews Monotone Chain";
    o.chAlg = CH_ANDREWS;
    run_walker_and_CH(o);

    Logger(LOG_INFO) << "Random Walk, Andrews Monotone Chain and Akl Toussaint";
    o.chAlg = CH_ANDREWS_AKL;
    run_walker_and_CH(o);

    Logger(LOG_INFO) << "Random Walk, Jarvis March";
    o.chAlg = CH_JARVIS;
    run_walker_and_CH(o);

    Logger(LOG_INFO) << "Random Walk, Jarvis March and Akl Toussaint";
    o.chAlg = CH_JARVIS_AKL;
    run_walker_and_CH(o);

    Logger(LOG_INFO) << "Random Walk, QHull";
    o.chAlg = CH_QHULL;
    run_walker_and_CH(o);

    Logger(LOG_INFO) << "Random Walk, QHull and Akl Toussaint";
    o.chAlg = CH_QHULL_AKL;
    run_walker_and_CH(o);


    o.steps = 10000;
    o.type = WT_LOOP_ERASED_RANDOM_WALK;
    o.benchmark_L = 4064.59205479;
    o.benchmark_A = 481541;

    Logger(LOG_INFO) << "Loop Erased Random Walk, Andrews Monotone Chain";
    o.chAlg = CH_ANDREWS;
    run_walker_and_CH(o);

    Logger(LOG_INFO) << "Loop Erased Random Walk, Andrews Monotone Chain and Akl Toussaint";
    o.chAlg = CH_ANDREWS_AKL;
    run_walker_and_CH(o);

    Logger(LOG_INFO) << "Loop Erased Random Walk, Jarvis March";
    o.chAlg = CH_JARVIS;
    run_walker_and_CH(o);

    Logger(LOG_INFO) << "Loop Erased Random Walk, Jarvis March and Akl Toussaint";
    o.chAlg = CH_JARVIS_AKL;
    run_walker_and_CH(o);

    Logger(LOG_INFO) << "Loop Erased Random Walk, QHull";
    o.chAlg = CH_QHULL;
    run_walker_and_CH(o);

    Logger(LOG_INFO) << "Loop Erased Random Walk, QHull and Akl Toussaint";
    o.chAlg = CH_QHULL_AKL;
    run_walker_and_CH(o);


    Logger(LOG_TIMING) << "Mem: " << vmPeak();

    //~ o.iterations = 1;
    //~ o.theta = 1;
}
