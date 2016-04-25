#pragma once

#include "Logging.hpp"
#include "RealWalker.hpp"

/** Draw the x, y, z, ... displacements from Gaussian distributions at each step.
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

        const std::vector<Step<double>>& steps() const;

        void change(UniformRNG &rng);
        void undoChange();

    protected:
        Step<double> genStep(std::vector<double>::iterator first) const;
};
