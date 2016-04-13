#pragma once

#include "Simulation.hpp"
#include "stat.hpp"

class Metropolis : public Simulation
{
    public:
        Metropolis(const Cmd &o);
        virtual void run();

    protected:
        int equilibrate(std::unique_ptr<Walker>& w1, UniformRNG& rngMC1);
};
