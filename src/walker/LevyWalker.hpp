#pragma once

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** A Levy Flight, i.e., random uniform directions and Levy distributed distances.
 *
 * Random walk, which chooses at each step a random direction from
 * a uniform distribution and a step distance from a heavy tailed Levy
 * distribution.
 */
class LevyWalker final : public SpecWalker<double>
{
    public:
        LevyWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo);

        void updateSteps() final;

        void change(UniformRNG &rng) final;
        void undoChange() final;

    protected:
        Step<double> genStep(std::vector<double>::iterator first) const;

        std::vector<double> undo_values;
};
