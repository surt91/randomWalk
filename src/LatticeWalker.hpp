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
            init();
        }

        void updateSteps();

        void change(UniformRNG &rng);
        void undoChange();
};
