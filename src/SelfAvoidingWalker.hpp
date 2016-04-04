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
            LOG(LOG_DEBUG) << "Dimerization got the inital SAW";
            random_numbers = std::vector<double>(l.begin(), l.end());
        }

        virtual ~SelfAvoidingWalker() {};

        virtual void change(UniformRNG &rng);
        virtual void undoChange();

    protected:
        Step transform(Step &p, const std::vector<int> &m) const;
        void pivot(const int index, const int op);

        std::list<double> dim(int N);
        bool checkOverlapFree(std::list<double> &l) const;
};
