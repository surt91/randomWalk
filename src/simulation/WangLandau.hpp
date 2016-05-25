#pragma once

#include "Simulation.hpp"
#include "Histogram.hpp"

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
        virtual void run();

    protected:
        void findStart(std::unique_ptr<Walker>& w, double lb, double ub, UniformRNG& rng);

        double lnf_min;
        double flatness_criterion;
};
