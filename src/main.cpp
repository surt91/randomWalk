#include <string>
#include <ctime>

#include "Cmd.hpp"
#include "Logging.hpp"
#include "Walker.hpp"
#include "LoopErasedWalker.hpp"
#include "RNG.hpp"
#include "misc.hpp"
#include "LargeDeviation.hpp"

void benchmark(Cmd &o)
{
    clock_t before_rng = clock();
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
    w->hullPoints();
    log<LOG_INFO>("Steps :") << w->nSteps();
    log<LOG_INFO>("Area  :") << w->L();
    log<LOG_INFO>("Volume:") << w->A();
    log<LOG_TOO_MUCH>("Trajectory:") << w->points();
    if(!o.svg_path.empty())
        w->svg(o.svg_path, true);

    log<LOG_TIMING>("RNG: ") << time_diff(before_walker, before_rng);
    log<LOG_TIMING>("RW : ") << time_diff(before_ch, before_walker);
    log<LOG_TIMING>("CH : ") << time_diff(before_output, before_ch);
    log<LOG_TIMING>("OUT: ") << time_diff(clock(), before_output);
}

int main(int argc, char** argv)
{
    Cmd o(argc, argv);

    if(o.benchmark)
    {
        benchmark(o);
        return 0;
    }

    run(o);
}
