#pragma once

#include <unordered_map>

#include "Logging.hpp"
#include "LatticeWalker.hpp"

class LoopErasedWalker : public LatticeWalker
{
    public:
        LoopErasedWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : LatticeWalker(d, numSteps, rng, hull_algo)
        {
        };

        virtual ~LoopErasedWalker() {};

        virtual const std::vector<Step<int>>& steps() const;

        virtual int nRN() const;

        virtual void change(UniformRNG &rng);
        virtual void undoChange();

    protected:
        mutable int random_numbers_used;
};
