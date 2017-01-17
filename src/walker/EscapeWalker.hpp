#pragma once

#include <unordered_map>
#include <unordered_set>

#include "../Logging.hpp"
#include "../Graph.hpp"
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
 */
class EscapeWalker final : public SpecWalker<int>
{
    public:
        EscapeWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void reconstruct() final;

        void updateSteps() final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

    private:
        Step<int> newStep;
        std::unordered_set<Step<int>> occupied;

        Step<int> undo_step;
        bool checkOverlapFree(const std::vector<Step<int>> &l);

        bool escapable(const Step<int> next);
        void create();

        Graph g;
        std::unordered_map<int, Step<int>> map;
        int min, max, dif;
};
