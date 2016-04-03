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
            random_numbers = std::vector<double>(l.begin(), l.end());
        }

        virtual ~SelfAvoidingWalker() {};

        virtual double rnChange(const int idx, const double other);

    protected:
        std::list<double> dim(int N);
        bool checkOverlapFree(std::list<double> &l) const;
};
