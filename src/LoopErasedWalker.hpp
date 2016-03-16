#pragma once

#include <unordered_map>

#include "Walker.hpp"

class LoopErasedWalker : public Walker
{
    public:
        LoopErasedWalker(int d, std::vector<double> &random_numbers)
            : Walker(d, random_numbers)
        {
            numSteps = -1;
        };

        virtual ~LoopErasedWalker() {};

        virtual const std::vector<Step> steps() const;
};
