#pragma once

#include "Simulation.hpp"
#include "../stat/RollingMean.hpp"

/** Standard Metropolis Monte Carlo sampling.
 *
 * Utilizes a large deviation scheme to simulate the random walks
 * at different "temperatures". In a postprocessing step, histograms
 * need to be created and stiched together. This will yield the
 * distribution over the full (or at least a large proportion of the)
 * support.
 *
 * See http://arxiv.org/pdf/1501.01041.pdf.
 * See also the Python scipts of this project.
 *
 * \image html temperatures.svg "SAWs at different 'temperatures'"
 */
class Metropolis : public Simulation
{
    public:
        Metropolis(const Cmd &o);
        virtual void run();

    protected:
        int equilibrate(std::unique_ptr<Walker>& w1, UniformRNG& rngMC1);
};
