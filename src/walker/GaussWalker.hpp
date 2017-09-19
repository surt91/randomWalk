#ifndef GAUSSWALKER_H
#define GAUSSWALKER_H

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** A walk with displacements drawn from a gaussian distribution (model for brownian motion).
 *
 * Draw the x, y, z, ... displacements from Gaussian distributions at each step.
 *
 * \image html GRW.svg "example of a gaussian random walk"
 */
class GaussWalker final : public SpecWalker<double>
{
    public:
        GaussWalker(int d, int numSteps, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void reconstruct() final;

        void updateSteps() final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        void changeSingle(UniformRNG &rng);
        void undoChangeSingle();

        void degenerateMaxVolume();

    protected:
        Step<double> genStep(std::vector<double>::iterator first) const;

        std::vector<double> undo_values;
};

#endif
