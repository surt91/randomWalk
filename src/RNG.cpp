#include "RNG.hpp"

std::vector<double> rng(int n, int seed)
{
    UniformRNG u(seed);
    return u.vector(n);
}

std::vector<double> UniformRNG::vector(int n)
{
    std::vector<double> v(n);
    std::generate(v.begin(), v.end(), uniform);

    return v;
}
