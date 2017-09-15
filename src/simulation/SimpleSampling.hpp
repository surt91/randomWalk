#pragma once

#include "Simulation.hpp"

/** Naive Simple Sampling
 *
 * This generates each sample fro scratch and performs "normal" simple sampling.
 * This can be faster for easy to generate realization as normal random walks,
 * but may be slower than Markov Chain Monte Carlo (eg. Metropolis) for, eg., SAW.
 */
class SimpleSampling : public Simulation
{
    public:
        SimpleSampling(const Cmd &o);
        virtual void run() override;
};
