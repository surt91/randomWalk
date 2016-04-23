#pragma once

#include <unordered_map>

#include "Logging.hpp"
#include "LatticeWalker.hpp"

/** Loop Erased Random Walk
 *
 * A Walk which does not self intersect.
 *
 * See also:
 * doi: 10.1.1.56.2276
 * [wiki](https://en.wikipedia.org/wiki/Loop-erased_random_walk)
 */
class LoopErasedWalker : public LatticeWalker
{
    public:
        LoopErasedWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : LatticeWalker(d, numSteps, rng, hull_algo)
        {
        }

        const std::vector<Step<int>>& steps() const;

        int nRN() const;

        void change(UniformRNG &rng);
        void undoChange();

    protected:
        mutable int random_numbers_used;
};
