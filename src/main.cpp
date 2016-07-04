#include <string>
#include <ctime>

#include "Cmd.hpp"
#include "Logging.hpp"
#include "RNG.hpp"
#include "walker/Walker.hpp"
#include "simulation/Metropolis.hpp"
#include "simulation/MetropolisParallelTempering.hpp"
#include "simulation/WangLandau.hpp"
#include "simulation/FastWangLandau.hpp"
#include "Benchmark.hpp"

/** randomWalk
 *
 * Program to gather the distrbution of different kinds of random walks
 * by different sampling techniques.
 */
int main(int argc, char** argv)
{
    Cmd o(argc, argv);

    if(o.benchmark)
    {
        return benchmark();
    }

    if(o.onlyBounds)
    {
        LOG(LOG_WARNING) << "mind that the following min/max values are ony rough estimates by a downhill algorithm";
        std::cout << "max: " << Simulation::getUpperBound(o) << std::endl;
        std::cout << "min: " << Simulation::getLowerBound(o) << std::endl;
        return 0;
    }

    if(o.onlyCenters)
    {
        WangLandau::printCenters(o);
        return 0;
    }

    if(o.sampling_method == SM_METROPOLIS)
    {
        Metropolis sim(o);
        sim.run();
    }
    else if(o.sampling_method == SM_METROPOLIS_PARALLEL_TEMPERING)
    {
        MetropolisParallelTempering sim(o);
        sim.run();
    }
    else if(o.sampling_method == SM_WANG_LANDAU)
    {
        WangLandau sim(o);
        sim.run();
    }
    else if(o.sampling_method == SM_FAST_WANG_LANDAU)
    {
        FastWangLandau sim(o);
        sim.run();
    }
    else
        LOG(LOG_ERROR) << "sampling method " << o.sampling_method << " is not known";
}
