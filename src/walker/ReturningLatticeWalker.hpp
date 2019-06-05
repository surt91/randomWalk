#ifndef RETURNINGLATTICEWALKER_H
#define RETURNINGLATTICEWALKER_H

#include <map>

#include "../Logging.hpp"
#include "../Permutation.hpp"
#include "SpecWalker.hpp"

/** Returning Random Walk on a Hypercubic lattice.
 *
 * Standard lattice random walk, with immediate reversals.
 * The lattice constant is unity. Will return to the start.
 *
 * \image html returning.svg "example of a returning random walk on a square lattice"
 */
class ReturningLatticeWalker final : public SpecWalker<int>
{
    public:
        ReturningLatticeWalker(int d, int numSteps, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void reconstruct() final;
        void updateSteps() final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

    private:
        Step<int> newStep;
        int undo_swap;

        Permutation permutation;
};

#endif
