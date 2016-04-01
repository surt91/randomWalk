#pragma once

#include <list>
#include <unordered_set>

#include "Logging.hpp"
#include "Walker.hpp"

class SelfAvoidingWalker : public Walker
{
    public:
        SelfAvoidingWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : Walker(d, numSteps, rng, hull_algo)
        {
            auto l(dim(numSteps));
            m_steps = std::vector<Step>(l.begin(), l.end());
            stepsDirty = false;
        };

        virtual ~SelfAvoidingWalker() {};

        virtual const std::vector<Step> steps() const;

        virtual double rnChange(const int idx, const double other);

    protected:
        std::list<Step> dim(int N);
        bool checkOverlapFree(std::list<Step> &l) const;
};
