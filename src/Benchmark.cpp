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
    std::string cmd("grep VmPeak /proc/"+pid+"/status");
    // or do I need "VmHWM" (high water mark)?
    return exec(cmd.c_str());
}

std::unique_ptr<Walker> run_walker(Cmd o)
{
    clock_t before_walker = clock();
    std::unique_ptr<Walker> w;
    UniformRNG rng(o.seedRealization);
    if(o.type == WT_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new Walker(o.d, o.steps, rng, o.chAlg));
    else if(o.type == WT_LOOP_ERASED_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, o.steps, rng, o.chAlg));
    else if(o.type == WT_SELF_AVOIDING_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new SelfAvoidingWalker(o.d, o.steps, rng, o.chAlg));
    w->steps();

    LOG(LOG_TIMING) << "RW : " << time_diff(before_walker, clock());

    return w;
}

void run_hull(Cmd o, std::unique_ptr<Walker> &w)
{
    clock_t before_ch = clock();
    w->setHullAlgo(o.chAlg);
    w->convexHull();

    clock_t before_output = clock();

    if(std::abs(w->L() - o.benchmark_L) > 0.01)
    {
        LOG(LOG_ERROR) << "Wrong L  " << w->L();
        LOG(LOG_ERROR) << "expected " << o.benchmark_L;
    }
    if(std::abs(w->A() - o.benchmark_A) > 1)
    {
        LOG(LOG_ERROR) << "Wrong A  " << w->A();
        LOG(LOG_ERROR) << "expected " << o.benchmark_A;
    }


    LOG(LOG_TIMING) << "CH : " << time_diff(before_ch, before_output);
}

void run_MC_simulation(Cmd /*o*/)
{
}

void benchmark()
{
    Logger::verbosity = LOG_TIMING;
    //~ Logger::verbosity = LOG_TOO_MUCH;

    Cmd o;
    o.benchmark = true;
    o.seedRealization = 13;
    o.seedMC = 42;
    o.d = 2;

    for(int i=1; i<=3; ++i)
    {
        switch(i)
        {
            case WT_RANDOM_WALK:
                o.steps = 1000000;
                o.type = WT_RANDOM_WALK;
                o.benchmark_L = 3747.18;
                o.benchmark_A = 984338;
                break;
            case WT_LOOP_ERASED_RANDOM_WALK:
                o.steps = 10000;
                o.type = WT_LOOP_ERASED_RANDOM_WALK;
                o.benchmark_L = 4064.59205479;
                o.benchmark_A = 481541;
                break;
            case WT_SELF_AVOIDING_RANDOM_WALK:
                o.steps = 2000;
                o.type = WT_SELF_AVOIDING_RANDOM_WALK;
                o.benchmark_L = 478.33;
                o.benchmark_A = 10287;
                break;
        }

        LOG(LOG_INFO) << TYPE_LABEL[i];
        auto w = run_walker(o);

        for(int j=1; j<=8; ++j)
        {
            o.chAlg = (hull_algorithm_t) j;
            LOG(LOG_INFO) << CH_LABEL[j];
            try{
                run_hull(o, w);
            } catch(...) { }
        }
    }

    LOG(LOG_TIMING) << "Mem: " << vmPeak();

    //~ o.iterations = 1;
    //~ o.theta = 1;
}
