#pragma once

#include <unordered_map>

#include "Logging.hpp"
#include "Walker.hpp"

class LoopErasedWalker : public Walker
{
    public:
        LoopErasedWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : Walker(d, numSteps, rng, hull_algo)
        {
        };

        virtual ~LoopErasedWalker() {};

        virtual const std::vector<Step> steps() const;

        virtual double rnChange(const int idx, const double other);
};
