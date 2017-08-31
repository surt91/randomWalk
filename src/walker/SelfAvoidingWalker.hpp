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
 *
 * \image html SAW.svg "example of a self-avoiding walk"
 */
class SelfAvoidingWalker final : public SpecWalker<int>
{
    public:
        SelfAvoidingWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void reconstruct() final;
        void generate_from_MCMC();
        void generate_from_dimerization();

        void updateSteps() final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        void degenerateMinSurface() final;

        void svgOfPivot(std::string filename);

    protected:
        Step<int> transform(const Step<int> &p, const int *m) const;
        bool pivot(const int index, const int op, bool update=true);

        int undo_naive_index;
        Step<int> undo_naive_step;
        Step<int> undo_step;
        int undo_symmetry;
        bool undo_slither_direction;
        bool naiveChange(const int idx, const double rn, bool update=true);
        void naiveChangeUndo();

        bool slitheringSnake(const bool front, const double rn, bool update=true);
        void undo_slitheringSnake();
        Step<int> slither(const bool front, const Step<int> &newStep);

        std::list<double> dim(int N);
        std::unordered_set<Step<int>> overlap_test;
        bool checkOverlapFree(const std::list<double> &l) const;
        bool checkOverlapFree(const std::vector<Step<int>> &l) const;
};
