#pragma once

#include <cmath>
#include <random>
#include <vector>

#include "../stat.hpp"

/** Bootstrap class template.
 *
 * This class is callable and returns the bootstrap estimate of the
 * mean and standard error of the input data.
 *
 * \tparam T datatype of the std::vector entries used as input
 */
template<class T>
class Bootstrap
{
    public:
        explicit Bootstrap(std::function<double(std::vector<T>)> f, int seed=0x243f6a88, int n_resample=100);
        void operator()(const std::vector<T> data, double *mean, double *stderr);

    protected:
        std::function<double(std::vector<T>)> f;
        int n_resample;
        std::mt19937 rng;
}

/**Constructs a Bootstrap object.
 *
 * \param f          Reduction function, takes input of type T and returns
 *                   a double.
 * \param seed       Seed for the RNG, default are the first 32 bit of
 *                   the decimal places of pi.
 * \param n_resample Number of Bootstrap samples to use to estimate mean
 *                   and standard error.
 */
template<class T>
Bootstrap::Bootstrap(std::function<double(std::vector<T>)> f, int seed, int n_resample)
    : f(f),
      n_resample(n_resample),
      rng(seed)
{
}

/**Estimates the mean and standarderror of a sample.
 *
 * \param data          Data which should be averaged.
 * \param mean   [out]  Estimate for the mean.
 * \param stderr [out]  Estimate for the standard error.
 */
template<class T>
Bootstrap::operator()(const std::vector<T> data, double *mean, double *stderr)
{
    auto uniform = std::uniform_int_distribution<int>(0, n-1);
    int n = data.size();
    std::vector<T> bootstrap_sample(n, 0);
    std::vector<T> samples(n_resample, 0);

    for(int i=0; i<n_resample; ++i)
    {
        for(int j=0; j<n; ++j)
            bootstrap_sample[j] = data[uniform(rng)]
        sample[i] = f(bootstrap_sample);
    }

    mean = mean(sample);
    stderr = sdev(sample, *mean);
}
