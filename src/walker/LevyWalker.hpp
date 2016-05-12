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
        LevyWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : SpecWalker<double>(d, numSteps, rng, hull_algo)
        {
            // we need d random numbers per step, for each angle one
            random_numbers = rng.vector(d * numSteps);
            // and for the distance a Levy (in this case Cauchy) distributed one
            for(int i=0; i<numSteps; ++i)
                random_numbers[i*d] = std::abs(rng.cauchy(1.));

            init();
        }
        ~LevyWalker() {}

        void updateSteps() final;

        void change(UniformRNG &rng) final;
        void undoChange() final;

    protected:
        Step<double> genStep(std::vector<double>::iterator first) const;

        std::vector<double> undo_values;
};
