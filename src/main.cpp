#include <string>
#include <ctime>

#include "Cmd.hpp"
#include "Logging.hpp"
#include "Walker.hpp"
#include "LoopErasedWalker.hpp"
#include "RNG.hpp"
#include "LargeDeviation.hpp"
#include "Benchmark.hpp"


int main(int argc, char** argv)
{
    Cmd o(argc, argv);

    if(o.benchmark)
    {
        benchmark();
        return 0;
    }

    if(o.sampling_method == SM_METROPOLIS)
        metropolis(o);
    else if(o.sampling_method == SM_WANG_LANDAU)
        wang_landau(o);
    else
        LOG(LOG_ERROR) << "sampling method " << o.sampling_method << " is not known";
}
