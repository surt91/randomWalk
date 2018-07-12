#include <string>
#include <ctime>

#include "Cmd.hpp"
#include "Logging.hpp"
#include "RNG.hpp"
#include "walker/Walker.hpp"
#include "walker/LoopErasedWalker.hpp"
#include "walker/SelfAvoidingWalker.hpp"
#include "simulation/Simulation.hpp"
#include "simulation/SimpleSampling.hpp"
#include "simulation/Metropolis.hpp"
#include "simulation/MetropolisParallelTempering.hpp"
#ifdef _MPI
#include "simulation/MetropolisParallelTemperingMPI.hpp"
#endif
#include "simulation/WangLandau.hpp"
#include "simulation/FastWLEntropic.hpp"

/** randomWalk
 *
 * Program to gather the distrbution of different kinds of random walks
 * by different sampling techniques.
 */
int main(int argc, char** argv)
{
    Cmd o(argc, argv);

    if(o.onlyBounds)
    {
        LOG(LOG_WARNING) << "mind that the following min/max values are only "
                            "rough estimates by a downhill algorithm";
        std::cout << "max: " << Simulation::getReasonalbleUpperBound(o) << std::endl;
        std::cout << "min: " << Simulation::getReasonalbleLowerBound(o) << std::endl;
        return 0;
    }

    if(o.onlyPivotExample)
    {
        LOG(LOG_INFO) << "generate a pivot example svg and exit";
        UniformRNG rngReal(o.seedRealization);
        SelfAvoidingWalker w(o.d, o.steps, rngReal, o.chAlg);
        w.svgOfPivot(o.svg_path);
        return 0;
    }

    if(o.onlyLERWExample)
    {
        LOG(LOG_INFO) << "generate a LERW svg and exit";
        UniformRNG rngReal(o.seedRealization);
        LoopErasedWalker w(o.d, o.steps, rngReal, o.chAlg);
        w.svgOfErasedLoops(o.svg_path);
        return 0;
    }

    if(o.onlyChangeExample)
    {
        LOG(LOG_INFO) << "generate a change svg and exit";
        UniformRNG rngReal(o.seedRealization);
        std::unique_ptr<Walker> w;
        o.sampling_method = SM_METROPOLIS;
        Simulation::prepare(w, o);
        w->svgOfChange(o.svg_path, rngReal);
        return 0;
    }

    if(o.onlySingleChangeExample)
    {
        LOG(LOG_INFO) << "generate n change svgs and exit";
        UniformRNG rngReal(o.seedRealization);
        std::unique_ptr<Walker> w;
        o.sampling_method = SM_METROPOLIS;
        Simulation::prepare(w, o);
        for(int i=0; i<o.iterations; ++i)
        {
            w->svg(o.svg_path + "." + std::to_string(i));
            w->change(rngReal);
        }
        return 0;
    }

    if(o.onlyPTTemperatures)
    {
        LOG(LOG_INFO) << "generate temperatures for parallel tempering and exit";
        while(true)
        {
            // TODO determine a t_eq automatically
            // use simple/OpenMP version
            MetropolisParallelTempering sim(o, !o.data_path_vector.empty());
            sim.run();
            auto newTemperatures = sim.proposeBetterTemperatures();
            LOG(LOG_INFO) << "new temperatures (#" << newTemperatures.size()
                          << "): [" << newTemperatures << "]";
            if(o.parallelTemperatures == newTemperatures)
                break;
            o.parallelTemperatures = std::move(newTemperatures);
        }
        // output as python dict entry, to copy into parameters.py
        std::cout << o.steps << ": [";
        for(const auto t : o.parallelTemperatures)
            std::cout << t << ", ";
        std::cout << "]," << std::endl;;
        return 0;
    }

    // TODO
    // if(o.onlyScentHistogram)
    // {
    //     LOG(LOG_INFO) << "generate a scent histogram svg and exit";
    //     UniformRNG rngReal(o.seedRealization);
    //     LoopErasedWalker w(o.d, o.steps, rngReal, o.chAlg);
    //     w.svgOfErasedLoops(o.svg_path);
    //     return 0;
    // }

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
#ifdef _MPI
    else if(o.sampling_method == SM_METROPOLIS_PARALLEL_TEMPERING_MPI)
    {
        MetropolisParallelTemperingMPI sim(o);
        sim.run();
    }
#endif
    else if(o.sampling_method == SM_WANG_LANDAU)
    {
        WangLandau sim(o);
        sim.run();
    }
    else if(o.sampling_method == SM_FAST_WANG_LANDAU)
    {
        FastWLEntropic sim(o);
        sim.run();
    }
    else if(o.sampling_method == SM_SIMPLESAMPLING)
    {
        SimpleSampling sim(o);
        sim.run();
    }
    else
        LOG(LOG_ERROR) << "sampling method " << o.sampling_method
                       << " is not known";
}
