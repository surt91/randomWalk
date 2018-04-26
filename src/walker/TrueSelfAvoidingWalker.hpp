#ifndef SELFAVOIDINGWALKER_H
#define SELFAVOIDINGWALKER_H

#include <list>
#include <iterator>
#include <unordered_set>

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** True Self-Avoiding Random Walk
 *
 * A Walk which tries not to self intersect.
 *
 * See also:
 * doi: 10.1103/PhysRevB.27.1635
 *
 * \image html TSAW.svg "example of a true self-avoiding walk"
 */
class TrueSelfAvoidingWalker final : public SpecWalker<int>
{
    public:
        TrueSelfAvoidingWalker(int d, int numSteps, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void reconstruct() final;

        void updateSteps() final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        double beta;
        void setP1(double beta) final;

    protected:
        Step<int> newStep;
        Step<int> undoStep;
};

#endif
