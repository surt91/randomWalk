#pragma once

#include "Simulation.hpp"

/** Metropolis Monte Carlo sampling enhanced with parallel tempering.
 */
class MetropolisParallelTempering : public Simulation
{
    public:
        MetropolisParallelTempering(const Cmd &o);
        virtual void run();

    protected:
        std::unordered_map<double, std::ofstream> mapThetaToFile;
        void sweep(std::unique_ptr<Walker> &w, double theta, UniformRNG &rngMC);
};
