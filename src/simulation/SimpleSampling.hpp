#pragma once

#include "Simulation.hpp"

class SimpleSampling : public Simulation
{
    public:
        SimpleSampling(const Cmd &o);
        virtual void run();
};
