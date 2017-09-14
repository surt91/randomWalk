#pragma once

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** Random Walk on a Hypercubic lattice.
 *
 * Standard lattice random walk, with immediate reversals.
 * Th lattice constant is unity.
 *
 * \image html LRW.svg "example of a random walk on a square lattice"
 */
class LatticeWalker final : public SpecWalker<int>
{
    public:
        LatticeWalker(int d, int numSteps, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void updateSteps() final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

    private:
        Step<int> newStep;
};
