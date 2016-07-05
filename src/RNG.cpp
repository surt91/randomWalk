#include "RNG.hpp"

std::vector<double> rng(int n, int seed)
{
    UniformRNG u(seed);
    return u.vector(n);
}

std::vector<double> UniformRNG::vector(int n)
{
    std::vector<double> v(n);
    for(auto &i : v)
        i = uniform();

    return v;
}

std::vector<double> UniformRNG::vector_gaussian(int n, const double mu, const double sigma)
{
    std::vector<double> v(n);
    for(auto &i : v)
        i = gaussian(mu, sigma);

    return v;
}

std::string UniformRNG::serialize_rng()
{
    std::stringstream ss;
    ss << rng;
    return std::string(ss.str());
}

void UniformRNG::deserialize_rng(std::string &s)
{
    std::stringstream ss;
    ss << s;
    ss >> rng;
}

/// Generates uniformly distributed random numbers
double UniformRNG::uniform()
{
    return std::uniform_real_distribution<double>(0.0, 1.0)(rng);
}

/// Generates normal distributed random numbers
double UniformRNG::gaussian(const double mu, const double sigma)
{
    return std::normal_distribution<double>(mu, sigma)(rng);
}

/** Generates a Levy distributed random number
 * This code is taken from the GSL (and slightly modified)
 *
 * http://www.gnu.org/software/gsl/
 */
double UniformRNG::levy(const double c, const double alpha)
{
    double u, v, t, s;

    u = M_PI * (uniform() - 0.5);

    // cauchy case
    if (alpha == 1)
    {
        t = tan (u);
        return c * t;
    }

    do
    {
        // exponentailly distributed random number
        // also from gsl
        v = -log1p(-uniform());
    }
    while (v == 0);

    // gaussian case
    if (alpha == 2)
    {
        t = 2 * sin (u) * sqrt(v);
        return c * t;
    }

  t = sin(alpha * u) / pow(cos(u), 1 / alpha);
  s = pow(cos ((1 - alpha) * u) / v, (1 - alpha) / alpha);

  return c * t * s;
}

/** Generates a Cauchy (Lorentz) distributed random number
 * This code is taken from the GSL (and slightly modified)
 *
 * http://www.gnu.org/software/gsl/
 */
double UniformRNG::cauchy(const double a)
{
    double u;
    do
    {
        u = uniform();
    }
    while (u == 0.5);

    return a * tan(M_PI * u);
}

/** Returns the engine.
 * can be useful for stl algorithms, like std::shuffle
 */
std::mt19937& UniformRNG::engine()
{
    return rng;
}
