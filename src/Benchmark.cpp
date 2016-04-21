#include "Benchmark.hpp"

/** Format time difference human readable.
 *
 * \param start is the timestamp at the beginning
 * \param end is the timestamp at the end
 * \return human readable time
 */
std::string time_diff(clock_t start, clock_t end)
{
    return std::to_string((double)(end - start) / CLOCKS_PER_SEC) + "s";
}

/** Executes the command and returns its standard out.
 *
 * taken from http://stackoverflow.com/a/478960/1698412
 *
 * \param cmd is the shell command to be executed
 * \return standard output of the command
 */
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

/** Yields the maximum vmem needed until the call to this function
 *
 * This function queries /proc/PID/status for VmPeak using a call
 * to grep. This is not very portable!
 *
 * \return string indicating VmPeak
 */
std::string vmPeak()
{
    std::string pid = std::to_string(getpid());
    std::string cmd("grep VmPeak /proc/"+pid+"/status");
    // or do I need "VmHWM" (high water mark)?
    return exec(cmd.c_str());
}

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

    w->updateSteps();

    LOG(LOG_TIMING) << "RW : " << time_diff(before_walker, clock());

    return w;
}

void run_hull(const Cmd &o, std::unique_ptr<Walker> &w)
{
    clock_t before_ch = clock();
    w->setHullAlgo(o.chAlg);
    w->updateHull();

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

void run_MC_simulation(const Cmd &o, std::unique_ptr<Walker> &w)
{
    UniformRNG rngMC(1);
    clock_t before_mc = clock();
    for(int i=0; i<o.iterations; ++i)
        w->change(rngMC);

    LOG(LOG_TIMING) << "MC : " << time_diff(before_mc, clock());
}

void benchmark()
{
    Logger::verbosity = LOG_TIMING;

    Cmd o;
    o.benchmark = true;
    o.seedRealization = 13;
    o.seedMC = 42;
    o.d = 2;

    o.simpleSampling = true;

    clock_t start = clock();

    for(int i=1; i<=3; ++i)
    {
        switch(i)
        {
            case WT_RANDOM_WALK:
                o.steps = 1000000;
                o.type = WT_RANDOM_WALK;
                o.benchmark_L = 3747.18;
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
                o.steps = 640;
                o.type = WT_SELF_AVOIDING_RANDOM_WALK;
                o.benchmark_L = 293.77;
                o.benchmark_A = 4252.5;
                o.iterations = 10000;
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

        run_MC_simulation(o, w);
    }

    LOG(LOG_TIMING) << "Total : " << time_diff(start, clock());
    LOG(LOG_TIMING) << "Mem: " << vmPeak();
}
