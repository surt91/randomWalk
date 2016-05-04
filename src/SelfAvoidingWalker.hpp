#pragma once

#include <list>
#include <iterator>
#include <unordered_set>

#include "Logging.hpp"
#include "SpecWalker.hpp"

/// transformation matrices for pivoting d=2
static const int tMatrix2[][4] =
    {
        // mirror at x-axis
            { 1,  0,
              0, -1},
        // mirror at y-axis
            {-1,  0,
              0,  1},
        // rotate by pi/2
            { 0,  1,
             -1,  0},
        // rotate by -pi/2
            { 0, -1,
              1,  0}
    };
/// inverse transformations
static const int iMatrix2[] =
    {
        0,
        1,
        3,
        2
    };

/// transformation matrices for pivoting d=3
static const int tMatrix3[][9] =
    {
        // mirror at xy-plane
            { 1,  0,  0,
              0,  1,  0,
              0,  0, -1},
        // mirror at zy-plane
            {-1,  0,  0,
              0,  1,  0,
              0,  0,  1},
        // mirror at xz-plane
            { 1,  0,  0,
              0, -1,  0,
              0,  0,  1},
        // rotate by pi/2 around x-axis
            { 1,  0,  0,
              0,  0,  1,
              0, -1,  0},
        // rotate by pi around x-axis
            { 1,  0,  0,
              0, -1,  0,
              0,  0, -1},
        // rotate by -pi/2 around x-axis
            { 1,  0,  0,
              0,  0, -1,
              0,  1,  0},
        // rotate by pi/2 around y-axis
            { 0,  0,  1,
              0,  1,  0,
             -1,  0,  0},
        // rotate by pi around y-axis
            {-1,  0,  0,
              0,  1,  0,
              0,  0, -1},
        // rotate by -pi/2 around y-axis
            { 0,  0, -1,
              0,  1,  0,
              1,  0,  0},
        // rotate by pi/2 around z-axis
            { 0, -1,  0,
              1,  0,  0,
              0,  0,  1},
        // rotate by pi around z-axis
            {-1,  0,  0,
              0, -1,  0,
              0,  0,  1},
        // rotate by -pi/2 around z-axis
            { 0,  1,  0,
             -1,  0,  0,
              0,  0,  1}
    };
/// inverse transformations
static const int iMatrix3[] =
    {
        0,
        1,
        2,
        5,
        4,
        3,
        8,
        7,
        6,
        11,
        10,
        9
    };

/** Self-Avoiding Random Walk
 *
 * A Walk which does not self intersect.
 *
 * See also:
 * doi: 10.1007/978-1-4614-6025-1_9
 * [wiki](https://en.wikipedia.org/wiki/Self-avoiding_walk)
 */
class SelfAvoidingWalker : public SpecWalker<int>
{
    public:
        SelfAvoidingWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : SpecWalker<int>(d, numSteps, rng, hull_algo)
        {
            auto l(dim(numSteps));
            random_numbers = std::vector<double>(l.begin(), l.end());
            init();
        }

        void updateSteps();

        void change(UniformRNG &rng);
        void undoChange();

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
