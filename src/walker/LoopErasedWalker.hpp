#ifndef LOOPERASEDWALKER_H
#define LOOPERASEDWALKER_H

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
 *
 * \image html LERW.svg "example of a loop erased random walk"
 */
class LoopErasedWalker final : public SpecWalker<int>
{
    public:
        LoopErasedWalker(int d, int numSteps, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void reconstruct() final;

        void updateSteps() final;

        int nRN() const final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        void svgOfErasedLoops(std::string filename);

    protected:
        mutable int random_numbers_used;
        Step<int> newStep;
        Step<int> undoStep;
};

#endif
