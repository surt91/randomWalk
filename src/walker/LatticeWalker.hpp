#pragma once

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** Random Walk on a Hypercubic lattice.
 *
 * Standard lattice random walk, with immediate reversals.
 * Th lattice constant is unity.
 */
class LatticeWalker final : public SpecWalker<int>
{
    public:
        LatticeWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo);

        void updateSteps() final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

    private:
        Step<int> newStep;
};
