#pragma once

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** A walk with displacements drawn from a gaussian distribution (model for brownian motion).
 *
 * Draw the x, y, z, ... displacements from Gaussian distributions at each step.
 */
class GaussWalker final : public SpecWalker<double>
{
    public:
        GaussWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo);

        void updateSteps() final;

        void change(UniformRNG &rng) final;
        void undoChange() final;

        void changeSingle(UniformRNG &rng);
        void undoChangeSingle();

        void degenerateMaxVolume();

    protected:
        Step<double> genStep(std::vector<double>::iterator first) const;

        std::vector<double> undo_values;
};
