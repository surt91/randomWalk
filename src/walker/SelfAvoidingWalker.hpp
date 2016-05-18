#pragma once

#include <list>
#include <iterator>
#include <unordered_set>

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** Self-Avoiding Random Walk
 *
 * A Walk which does not self intersect.
 *
 * See also:
 * doi: 10.1007/978-1-4614-6025-1_9
 * [wiki](https://en.wikipedia.org/wiki/Self-avoiding_walk)
 */
class SelfAvoidingWalker final : public SpecWalker<int>
{
    public:
        SelfAvoidingWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo);

        void updateSteps() final;

        void change(UniformRNG &rng) final;
        void undoChange() final;

    protected:
        Step<int> transform(Step<int> &p, const int *m) const;
        bool pivot(const int index, const int op);

        int undo_naive_index;
        Step<int> undo_naive_step;
        Step<int> undo_step;
        int undo_symmetry;
        bool naiveChange(const int idx, const double rn);
        void naiveChangeUndo();

        std::list<double> dim(int N);
        bool checkOverlapFree(const std::list<double> &l) const;
        bool checkOverlapFree(const std::vector<Step<int>> &l) const;
};
