#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <functional>

std::vector<double> rng(int n, int seed=0);

class UniformRNG
{
    public:
        UniformRNG(int seed)
            : rng(seed),
              distribution(0.0, 1.0),
              uniform(std::bind(distribution, rng))
        {}

        double operator()()
        {
            return uniform();
        }

        std::vector<double> vector(int n);

        std::mt19937 rng;
    protected:
        std::uniform_real_distribution<double>distribution;
        std::function<double()> uniform;
};
