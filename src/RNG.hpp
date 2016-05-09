#pragma once

#include <cmath>
#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include <sstream>

std::vector<double> rng(int n, int seed=0);

/** Wrapper for the random number generator.
 *
 * Uses a c++11 std Mersenne Twister generator, to yield random
 * numbers from different distributions.
 */
class UniformRNG
{
    protected:
        std::mt19937 rng;
        std::uniform_real_distribution<double> distribution;
        std::normal_distribution<double> distribution_gaussian;

    public:
        UniformRNG() {}

        UniformRNG(int seed)
            : rng(seed),
              distribution(0.0, 1.0),
              distribution_gaussian(0.0, 1.0),
              uniform(std::bind(distribution, rng)),
              gaussian(std::bind(distribution_gaussian, rng))
        {}

        double operator()()
        {
            return uniform();
        }

        std::vector<double> vector(int n);
        std::vector<double> vector_gaussian(int n);

        std::function<double()> uniform;
        std::function<double()> gaussian;

        double levy(const double c, const double alpha);
        double cauchy(const double a);

        std::string serialize_rng();
        void deserialize_rng(std::string &s);


};
