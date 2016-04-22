#pragma once

#include "Logging.hpp"
#include "SpecWalker.hpp"

/** Base class for all non-lattice walks.
 *
 * Random walk, which chooses at each step a random direction from
 * a uniform distribution and a constant distance 1.
 */
class RealWalker : public SpecWalker<double>
{
    public:
        RealWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : SpecWalker<double>(d, numSteps, rng, hull_algo)
        {
            // we need d-1 random numbers per step, for each angle one
            random_numbers = std::move(rng.vector((d-1) * numSteps));
        }
        virtual ~RealWalker() {}

        virtual const std::vector<Step<double>>& steps() const;

        virtual void change(UniformRNG &rng);
        virtual void undoChange();

        virtual void degenerateMaxVolume();
        virtual void degenerateMaxSurface();
        virtual void degenerateSpiral();
        virtual void degenerateStraight();

    protected:
        virtual Step<double> genStep(std::vector<double>::iterator first) const;

        std::vector<double> undo_values;
};
