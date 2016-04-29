#pragma once

#include "Logging.hpp"
#include "SpecWalker.hpp"

/** Base class for all lattice walks.
 *
 * Standard lattice random walk, with immediate reversals.
 */
class LatticeWalker : public SpecWalker<int>
{
    public:
        LatticeWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : SpecWalker<int>(d, numSteps, rng, hull_algo)
        {
            random_numbers = rng.vector(numSteps);
            updateSteps();
        }
        virtual ~LatticeWalker() {}

        virtual void updateSteps();

        virtual void change(UniformRNG &rng);
        virtual void undoChange();

        virtual void degenerateMaxVolume();
        virtual void degenerateMaxSurface();
        virtual void degenerateSpiral();
        virtual void degenerateStraight();

    protected:
};
