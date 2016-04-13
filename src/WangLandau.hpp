#pragma once

#include "Simulation.hpp"
#include "Histogram.hpp"
#include "DensityOfStates.hpp"

class WangLandau : public Simulation
{
    public:
        WangLandau(const Cmd &o);
        virtual void run();

    protected:
        double getLowerBound();
        double getUpperBound();
};
