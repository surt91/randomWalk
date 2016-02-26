#include "RNG.hpp"

std::vector<double> rng(int n, int seed)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    auto uniform = std::bind(distribution, rng);

    std::vector<double> v(n);
    std::generate(v.begin(), v.end(), uniform);

    return v;
}
