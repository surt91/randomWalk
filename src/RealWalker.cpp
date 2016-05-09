#include "RealWalker.hpp"

/** Generate a step by unit distance and angles determined by the
 * d-1 random numbers after first (inclusive first).
 *
 * Algortihm see http://en.wikipedia.org/wiki/N-sphere#Spherical_coordinates
 *
 * \param first iterator to the first random number to use, must have at
 *        least $d-1$ following entries
 * \return r-value Step
 */
Step<double> RealWalker::genStep(std::vector<double>::iterator first) const
{
    Step<double> s(d);

    // http://en.wikipedia.org/wiki/N-sphere#Spherical_coordinates
    double r = 1;
    std::vector<double> theta(d-1);
    for(int i=0; i<d-2; ++i)
        theta[i] = *first++ * M_PI;
    theta[d-2] = *first++ * 2*M_PI;

    for(int i=0; i<d-1; ++i)
    {
        s[i] = r;
        for(int j=0; j<i; ++j)
            s[i] *= sin(theta[j]);

        if(i == d-2)
            s[i+1] = s[i] * sin(theta[i]);

        s[i] *= cos(theta[i]);
    }
    return s;
}

void RealWalker::updateSteps()
{
    m_steps.resize(numSteps);
    for(int i=0; i<numSteps; ++i)
        m_steps[i] = genStep(random_numbers.begin() + i*(d-1));
}

void RealWalker::change(UniformRNG &rng)
{
    // We need d-1 random numbers per step to determine the d-1 angles
    int idx = rng() * numSteps;
    int rnidx = idx * (d-1);
    undo_index = idx;
    undo_values = std::vector<double>(random_numbers.begin() + rnidx, random_numbers.begin() + rnidx + d);
    for(int i=0; i<d-1; ++i)
        random_numbers[rnidx + i] = rng();

    m_steps[idx] = genStep(random_numbers.begin() + rnidx);
    updatePoints(idx+1);
    updateHull();
}

void RealWalker::undoChange()
{
    int t = 0;
    for(const auto i : undo_values)
        random_numbers[undo_index * (d-1) + t++] = i;

    m_steps[undo_index] = genStep(undo_values.begin());
    updatePoints(undo_index+1);
    updateHull();
}
