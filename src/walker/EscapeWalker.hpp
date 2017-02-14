#pragma once

#include <unordered_map>
#include <unordered_set>

#include "../Logging.hpp"
#include "../Hypercube.hpp"
// #include "../Graph.hpp"
#include "SpecWalker.hpp"

/** Escape Walk on a Hypercubic lattice.
 *
 * This is a walk on a lattice which may not intersect itself.
 * If the next step would intersect, a new random step replaces this
 * step. Also if after the next step an infinite point is not reachable
 * anymore, the step is replaced by a random step.
 *
 * Note that this type has different behavior than SAW. I am not sure if
 * it was studied before, though it is often mentioned as the naive
 * SAW implementation, which has different properties.
 *
 * In the literature it is known as the smart kinetic self-avoiding walk (SKSAW):
 * 10.1007/s10955-015-1271-4
 *
 * The exponent nu is expected to be 4/7.
 * 10.1103/PhysRevLett.59.539
 */
class EscapeWalker final : public SpecWalker<int>
{
    public:
        EscapeWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void reconstruct() final;

        void updateSteps() final;

        int nRN() const final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

    protected:
        Step<int> newStep;
        Step<int> undoStep;
        std::unordered_set<Step<int>> occupied;

        Step<int> undo_step;

        bool escapable(const Step<int> &next, const Step<int> &current);

        mutable int random_numbers_used;

        Hypercube g;
};
