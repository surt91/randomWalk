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
        RealWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void updateSteps() final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

    protected:
        Step<double> genStep(std::vector<double>::iterator first) const;

        std::vector<double> undo_values;
};
