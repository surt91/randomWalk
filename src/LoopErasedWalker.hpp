#pragma once

#include <unordered_map>

#include "Logging.hpp"
#include "Walker.hpp"

class LoopErasedWalker : public Walker
{
    public:
        LoopErasedWalker(int d, std::vector<double> &random_numbers, hull_algorithm_t hull_algo)
            : Walker(d, random_numbers, hull_algo)
        {
            numSteps = -1;
        };

        virtual ~LoopErasedWalker() {};

        virtual const std::vector<Step> steps(int limit=0) const;
};
