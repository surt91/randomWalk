#include <string>
#include <ctime>

#include "Cmd.hpp"
#include "Logging.hpp"
#include "Walker.hpp"
#include "RNG.hpp"
#include "Metropolis.hpp"
#include "WangLandau.hpp"
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
    {
        Metropolis sim(o);
        sim.run();
    }
    else if(o.sampling_method == SM_WANG_LANDAU)
    {
        WangLandau sim(o);
        sim.run();
    }
    else
        LOG(LOG_ERROR) << "sampling method " << o.sampling_method << " is not known";
}
