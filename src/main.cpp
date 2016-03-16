#include <string>
#include <ctime>

#include "Cmd.hpp"
#include "Logging.hpp"
#include "Walker.hpp"
#include "LoopErasedWalker.hpp"
#include "RNG.hpp"
#include "ConvexHull.hpp"
#include "misc.hpp"

int main(int argc, char** argv)
{
    Cmd o(argc, argv);

    clock_t before_rng = clock();
    std::vector<double> numbers = rng(o.n, o.seed);

    clock_t before_walker = clock();
    std::unique_ptr<Walker> w;
    if(o.type == 1)
        w = std::unique_ptr<Walker>(new Walker(o.d, numbers));
    else if(o.type == 2)
        w = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, numbers));
    w->steps();

    clock_t before_ch = clock();
    w->convexHull();

    clock_t before_output = clock();
    w->hullPoints();
    log<LOG_INFO>("Steps :") << w->nSteps();
    log<LOG_INFO>("Area  :") << w->L();
    log<LOG_INFO>("Volume:") << w->A();
    log<LOG_TOO_MUCH>("Trajectory:") << w->print();
    if(!o.svg_path.empty())
        w->svg(o.svg_path, true);

    log<LOG_TIMING>("RNG: ") << time_diff(before_walker, before_rng);
    log<LOG_TIMING>("RW : ") << time_diff(before_ch, before_walker);
    log<LOG_TIMING>("CH : ") << time_diff(before_output, before_ch);
    log<LOG_TIMING>("OUT: ") << time_diff(clock(), before_output);
}
