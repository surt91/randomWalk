#pragma once

#include <unordered_map>

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** Loop Erased Random Walk
 *
 * A Walk which does not self intersect.
 *
 * See also:
 * doi: 10.1.1.56.2276
 * [wiki](https://en.wikipedia.org/wiki/Loop-erased_random_walk)
 */
class LoopErasedWalker final : public SpecWalker<int>
{
    public:
        LoopErasedWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : SpecWalker<int>(d, numSteps, rng, hull_algo)
        {
            random_numbers = rng.vector(numSteps);
            init();
        }

        void updateSteps() final;

        int nRN() const final;

        void change(UniformRNG &rng) final;
        void undoChange() final;

    protected:
        mutable int random_numbers_used;
};
