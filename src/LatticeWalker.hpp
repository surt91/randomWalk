#pragma once

#include "Logging.hpp"
#include "SpecWalker.hpp"

class LatticeWalker : public SpecWalker<int>
{
    public:
        LatticeWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : SpecWalker<int>(d, numSteps, rng, hull_algo)
        {
        };

        virtual void change(UniformRNG &rng);
        virtual void undoChange();

        virtual void degenerateMaxVolume();
        virtual void degenerateMaxSurface();
        virtual void degenerateSpiral();
        virtual void degenerateStraight();

    protected:
};
