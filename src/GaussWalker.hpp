#pragma once

#include "Logging.hpp"
#include "RealWalker.hpp"

/** Base class for all non-lattice walks.
 *
 * Random walk, which chooses at each step a random direction from
 * a uniform distribution and a constant distance 1.
 */
class GaussWalker : public RealWalker
{
    public:
        GaussWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : RealWalker(d, numSteps, rng, hull_algo)
        {
            // we need d gaussian random numbers per step, for each direction
            random_numbers = rng.vector_gaussian(d * numSteps);
        }
        virtual ~GaussWalker() {}

        virtual const std::vector<Step<double>>& steps() const;

        virtual void change(UniformRNG &rng);
        virtual void undoChange();

    protected:
        virtual Step<double> genStep(std::vector<double>::iterator first) const;
};
