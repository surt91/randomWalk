#pragma once

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** A walk with fixed step length but random direction.
 *
 * Random walk, which chooses at each step a random direction from
 * a uniform distribution and a constant distance 1.
 */
class RealWalker final : public SpecWalker<double>
{
    public:
        RealWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : SpecWalker<double>(d, numSteps, rng, hull_algo)
        {
            // we need d-1 random numbers per step, for each angle one
            random_numbers = rng.vector((d-1) * numSteps);
            init();
        }
        ~RealWalker() {}

        void updateSteps() final;

        void change(UniformRNG &rng) final;
        void undoChange() final;

    protected:
        Step<double> genStep(std::vector<double>::iterator first) const;

        std::vector<double> undo_values;
};
