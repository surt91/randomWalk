#include "CorrelatedWalker.hpp"

CorrelatedWalker::CorrelatedWalker(int d, int numSteps, const UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<double>(d, numSteps, rng_in, hull_algo, amnesia),
      mu(0.0),
      sigma(1.0)
{
    // we need d random numbers per step, for each angle difference one and a distance
    // we generate d per step and overwrite unnecessary ones afterwards
    // TODO: replace Gaussian by wrapped normal
    random_numbers = rng.vector_gaussian(d * numSteps);
    // and for the distance a uniformly distributed one
    for(int i=0; i<numSteps; ++i)
        random_numbers[i*d] = rng.uniform();

    init();
}

/// Get new random numbers and reconstruct the walk
void CorrelatedWalker::reconstruct()
{
    // write new random numers into our state
    // we need d random numbers per step, for each angle difference one and a distance
    // we generate d per step and overwrite unnecessary ones afterwards
    // TODO: replace Gaussian by wrapped normal
    std::generate(random_numbers.begin(), random_numbers.end(),
                  [this]{ return this->rng.gaussian(); });
    // and for the distance a uniformly distributed one
    for(int i=0; i<numSteps; ++i)
        random_numbers[i*d] = rng.uniform();
    init();
}

/** Generate a step by distance \f$\in [0,1)\f$ and \f$d-1\f$ gaussian angles differences.
 *
 * Algortihm see http://en.wikipedia.org/wiki/N-sphere#Spherical_coordinates
 *
 * \param first iterator to the first random number to use, must have at
 *        least \f$d\f$ following entries
 * \return Step
 */
Step<double> CorrelatedWalker::genStep(std::vector<double>::iterator first) const
{
    Step<double> s(d);

    for(int i=0; i<d; ++i)
        s[i] = *first++ * sigma + mu;

    return s;
}

void CorrelatedWalker::setP1(double p1)
{
    // this is expensive, so ask first, if something changes
    if(mu == p1)
        return;
    mu = p1;
    updateSteps();
    updatePoints();
    updateHull();
}

void CorrelatedWalker::setP2(double p2)
{
    // this is expensive, so ask first, if something changes
    if(sigma == p2)
        return;
    sigma = p2;
    updateSteps();
    updatePoints();
    updateHull();
}

void CorrelatedWalker::updateSteps()
{
    m_steps.resize(numSteps);
    for(int i=0; i<numSteps; ++i)
        m_steps[i] = genStep(random_numbers.begin() + i*d);
}

void CorrelatedWalker::updatePoints(int /*start*/)
{
    Step<double> s(d);
    std::vector<double> theta(d-1, 0);

    // we can not start at 'start' because we need to add up all angles
    for(int i=1; i<=numSteps; ++i)
    {
        m_points[i].setZero();
        m_points[i] += m_points[i-1];

        // http://en.wikipedia.org/wiki/N-sphere#Spherical_coordinates
        double r = m_steps[i-1][0];

        // the angle is always added to
        for(int k=0; k<d-2; ++k)
            theta[k] += m_steps[i-1][k+1] / 2; // because only from 0 to pi
        theta[d-2] += m_steps[i-1][d-1];

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

void CorrelatedWalker::change(UniformRNG &rng, bool update)
{
    // We need d random numbers per step to determine the d-1 angles and distance
    int idx = rng() * numSteps;
    int rnidx = idx * d;
    undo_index = idx;
    undo_values = std::vector<double>(random_numbers.begin() + rnidx,
                                      random_numbers.begin() + rnidx + d);
    random_numbers[rnidx] = rng.uniform();
    for(int i=1; i<d; ++i)
        random_numbers[rnidx + i] = rng.gaussian();

    m_steps[idx] = genStep(random_numbers.begin() + rnidx);
    updatePoints(idx+1);

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

void CorrelatedWalker::undoChange()
{
    int t = 0;
    for(const auto i : undo_values)
        random_numbers[undo_index * d + t++] = i;

    m_steps[undo_index] = genStep(undo_values.begin());
    updatePoints(undo_index+1);
    m_convex_hull = m_old_convex_hull;
}
