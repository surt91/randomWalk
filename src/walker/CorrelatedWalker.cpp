#include "CorrelatedWalker.hpp"

/** Generate a step by distance $\in [0,1)$ and d-1 gaussian angles differences.
 *
 * Algortihm see http://en.wikipedia.org/wiki/N-sphere#Spherical_coordinates
 *
 * \param first iterator to the first random number to use, must have at
 *        least $d$ following entries
 * \return Step
 */
Step<double> CorrelatedWalker::genStep(std::vector<double>::iterator first) const
{
    Step<double> s(d);

    for(int i=0; i<d; ++i)
        s[i] = *first++;

    return s;
}

void CorrelatedWalker::updateSteps()
{
    m_steps.resize(numSteps);
    for(int i=0; i<numSteps; ++i)
        m_steps[i] = genStep(random_numbers.begin() + i*d);
}

void CorrelatedWalker::updatePoints(int start)
{
    Step<double> s(d);
    std::vector<double> theta(d-1);

    for(int i=start; i<=numSteps; ++i)
    {
        m_points[i].setZero();
        m_points[i] += m_points[i-1];

        // http://en.wikipedia.org/wiki/N-sphere#Spherical_coordinates
        double r = m_steps[i-1][0];
        for(int k=0; k<d-2; ++k)
            theta[k] = m_steps[i-1][k+1] * M_PI;
        theta[d-2] = m_steps[i-1][d-1] * 2*M_PI;

        for(int k=0; k<d-1; ++k)
        {
            s[k] = r;
            for(int j=0; j<k; ++j)
                s[k] *= sin(theta[j]);

            if(k == d-2)
                s[k+1] = s[k] * sin(theta[k]);

            s[k] *= cos(theta[k]);
        }

        m_points[i] += s;
    }
}

void CorrelatedWalker::change(UniformRNG &rng)
{
    // We need d random numbers per step to determine the d-1 angles and distance
    int idx = rng() * numSteps;
    int rnidx = idx * d;
    undo_index = idx;
    undo_values = std::vector<double>(random_numbers.begin() + rnidx, random_numbers.begin() + rnidx + d);
    for(int i=0; i<d; ++i)
        random_numbers[rnidx + i] = rng();

    m_steps[idx] = genStep(random_numbers.begin() + rnidx);
    updatePoints(idx+1);
    updateHull();
}

void CorrelatedWalker::undoChange()
{
    int t = 0;
    for(const auto i : undo_values)
        random_numbers[undo_index * d + t++] = i;

    m_steps[undo_index] = genStep(undo_values.begin());
    updatePoints(undo_index+1);
    updateHull();
}
