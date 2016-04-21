#pragma once

#include <list>
#include <iterator>
#include <unordered_set>

#include "Logging.hpp"
#include "LatticeWalker.hpp"

// transformation matrices for pivoting d=2
static const std::vector<std::vector<int>> tMatrix2 =
    {
        // mirror at x-axis
        std::vector<int>(
            { 1,  0,
              0, -1}
        ),
        // mirror at y-axis
        std::vector<int>(
            {-1,  0,
              0,  1}
        ),
        // rotate by pi/2
        std::vector<int>(
            { 0,  1,
             -1,  0}
        ),
        // rotate by -pi/2
        std::vector<int>(
            { 0, -1,
              1,  0}
        )
    };
// inverse transformations
static const std::vector<int> iMatrix2 =
    {
        0,
        1,
        3,
        2
    };

// transformation matrices for pivoting d=3
static const std::vector<std::vector<int>> tMatrix3 =
    {
        // mirror at xy-plane
        std::vector<int>(
            { 1,  0,  0,
              0,  1,  0,
              0,  0, -1}
        ),
        // mirror at zy-plane
        std::vector<int>(
            {-1,  0,  0,
              0,  1,  0,
              0,  0,  1}
        ),
        // mirror at xz-plane
        std::vector<int>(
            { 1,  0,  0,
              0, -1,  0,
              0,  0,  1}
        ),
        // rotate by pi/2 around x-axis
        std::vector<int>(
            { 1,  0,  0,
              0,  0,  1,
              0, -1,  0}
        ),
        // rotate by pi around x-axis
        std::vector<int>(
            { 1,  0,  0,
              0, -1,  0,
              0,  0, -1}
        ),
        // rotate by -pi/2 around x-axis
        std::vector<int>(
            { 1,  0,  0,
              0,  0, -1,
              0,  1,  0}
        ),
        // rotate by pi/2 around y-axis
        std::vector<int>(
            { 0,  0,  1,
              0,  1,  0,
             -1,  0,  0}
        ),
        // rotate by pi around y-axis
        std::vector<int>(
            {-1,  0,  0,
              0,  1,  0,
              0,  0, -1}
        ),
        // rotate by -pi/2 around y-axis
        std::vector<int>(
            { 0,  0, -1,
              0,  1,  0,
              1,  0,  0}
        ),
        // rotate by pi/2 around z-axis
        std::vector<int>(
            { 0, -1,  0,
              1,  0,  0,
              0,  0,  1}
        ),
        // rotate by pi around z-axis
        std::vector<int>(
            {-1,  0,  0,
              0, -1,  0,
              0,  0,  1}
        ),
        // rotate by -pi/2 around z-axis
        std::vector<int>(
            { 0,  1,  0,
             -1,  0,  0,
              0,  0,  1}
        )
    };
// inverse transformations
static const std::vector<int> iMatrix3 =
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

class SelfAvoidingWalker : public LatticeWalker
{
    public:
        SelfAvoidingWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : LatticeWalker(d, numSteps, rng, hull_algo)
        {
            auto l(dim(numSteps));
            LOG(LOG_DEBUG) << "Dimerization got the inital SAW";
            random_numbers = std::vector<double>(l.begin(), l.end());
        }

        virtual void change(UniformRNG &rng);
        virtual void undoChange();

    protected:
        Step<int> transform(Step<int> &p, const std::vector<int> &m) const;
        bool pivot(const int index, const int op);

        int undo_naive_index;
        Step<int> undo_naive_step;
        Step<int> undo_step;
        bool naiveChange(const int idx, const double rn);
        void naiveChangeUndo();

        std::list<double> dim(int N);
        bool checkOverlapFree(std::list<double> &l) const;
        bool checkOverlapFree(std::vector<Step<int>> &l) const;
};
