#pragma once

#include "Simulation.hpp"
#include "Histogram.hpp"
#include "DensityOfStates.hpp"

/** Wang Landau Sampling of the distribution of a given observable.
 *
 * Wang Landau sampling will sample the distribution over the full
 * support. Takes usually longer than Metropolis sampling, but does
 * not need parameters like temperatures or explicit equilibration.
 *
 * See http://arxiv.org/pdf/cond-mat/0011174.pdf
 */
class WangLandau : public Simulation
{
    public:
        WangLandau(const Cmd &o);
        virtual ~WangLandau() {}
        virtual void run();

    protected:
        double getLowerBound();
        double getUpperBound();
};
