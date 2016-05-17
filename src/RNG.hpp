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

    public:
        UniformRNG() {}

        UniformRNG(int seed)
            : rng(seed)
        {}

        double operator()()
        {
            return uniform();
        }

        std::vector<double> vector(int n);
        std::vector<double> vector_gaussian(int n, const double mu=0., const double sigma=1.);

        double uniform();
        double gaussian(const double mu=0., const double sigma=1.);
        double levy(const double c, const double alpha);
        double cauchy(const double a=1.);

        std::string serialize_rng();
        void deserialize_rng(std::string &s);


};
