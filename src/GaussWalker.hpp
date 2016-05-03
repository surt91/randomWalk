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
            updateSteps();
            updatePoints();
        }

        virtual void updateSteps();

        void change(UniformRNG &rng);
        void undoChange();

        void changeSingle(UniformRNG &rng);
        void undoChangeSingle();

        virtual void degenerateMaxVolume();

    protected:
        Step<double> genStep(std::vector<double>::iterator first) const;
};
