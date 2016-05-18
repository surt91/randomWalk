#pragma once

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** A correlated random walk, i.e., the next directions depends on the previous.
 *
 * Random walk, which chooses at each step a random direction difference
 * $\Delta \theta$ from a wrapped normal distribution and a distance
 * from a uniform distribution.
 *
 * This class behaves abit different than the other walkers. Since
 * this is not a Markov Process, ::steps does not contain steps, but
 * a stepsize and d-1 angles which are converted to proper points in the
 * updatePoints() function.
 */
class CorrelatedWalker final : public SpecWalker<double>
{
    public:
        CorrelatedWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo);

        void updateSteps() final;
        void updatePoints(int start=1) final;

        void change(UniformRNG &rng) final;
        void undoChange() final;

    protected:
        Step<double> genStep(std::vector<double>::iterator first) const;

        std::vector<double> undo_values;
};
