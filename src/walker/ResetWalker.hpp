#ifndef RESETWALKER_H
#define RESETWALKER_H

#include <list>
#include <iterator>
#include <unordered_set>

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** Resetting Random Walk
 *
 * A Walk which resets with probability p.
 *
 * See also:
 * doi: 10.1103/PhysRevLett.106.160601
 *
 * \image html RRW.svg "example of a resetting random walk, \f$p = 0.1\f$"
 */
class ResetWalker final : public SpecWalker<int>
{
    public:
        ResetWalker(int d, int numSteps, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void reconstruct() final;

        void updateSteps() final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        double resetrate;
        void setP1(double beta) final;
};

#endif
