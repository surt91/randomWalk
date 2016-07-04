#pragma once

#include "Simulation.hpp"

/** Metropolis Monte Carlo sampling enhanced with parallel tempering.
 */
class MetropolisParallelTempering : public Simulation
{
    public:
        virtual ~MetropolisParallelTempering() {};
        MetropolisParallelTempering(const Cmd &o);
        virtual void run();

    protected:
        void sweep(std::unique_ptr<Walker> &w, double theta, UniformRNG &rngMC);
};
