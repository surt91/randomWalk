#pragma once

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** A walk with displacements drawn from a gaussian distribution (model for brownian motion).
 *
 * Draw the x, y, z, ... displacements from Gaussian distributions at each step.
 */
class GaussWalker : public SpecWalker<double>
{
    public:
        GaussWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : SpecWalker<double>(d, numSteps, rng, hull_algo)
        {
            // we need d gaussian random numbers per step, for each direction
            random_numbers = rng.vector_gaussian(d * numSteps);
            init();
        }

        void updateSteps();

        void change(UniformRNG &rng);
        void undoChange();

        void changeSingle(UniformRNG &rng);
        void undoChangeSingle();

        void degenerateMaxVolume();

    protected:
        Step<double> genStep(std::vector<double>::iterator first) const;

        std::vector<double> undo_values;
};
