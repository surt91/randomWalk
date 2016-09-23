#include "LevyWalker.hpp"

LevyWalker::LevyWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
    : SpecWalker<double>(d, numSteps, rng, hull_algo)
{
    // we need d random numbers per step, for each angle one
    random_numbers = rng.vector(d * numSteps);
    // and for the distance a Levy (in this case Cauchy) distributed one
    for(int i=0; i<numSteps; ++i)
        random_numbers[i*d] = std::abs(rng.cauchy(1.));

    init();
}

void LevyWalker::reconstruct()
{
    // write new gaussian random numers into our state
    std::generate(random_numbers.begin(), random_numbers.end(), [this]{ return this->rng(); });
    for(int i=0; i<numSteps; ++i)
        random_numbers[i*d] = std::abs(rng.cauchy(1.));
    init();
}


/** Generate a step with a Levy distributed distance and angles determined by the
 * d random numbers after first (inclusive first).
 *
 * Mind that first needs to contain a mix of levy and uniformly distributed
 * random numbers.
 *
 * \param first iterator to the first random number to use, must have at
 *        least $d$ following entries
 * \return Step
 */
Step<double> LevyWalker::genStep(std::vector<double>::iterator first) const
{
    Step<double> s(d);

    // http://en.wikipedia.org/wiki/N-sphere#Spherical_coordinates
    double r = *first++;
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

void LevyWalker::updateSteps()
{
    m_steps.resize(numSteps);
    for(int i=0; i<numSteps; ++i)
        m_steps[i] = genStep(random_numbers.begin() + i*(d-1));
}

void LevyWalker::change(UniformRNG &rng, bool update)
{
    // We need d random numbers per step
    // d-1 uniformly distributed to determine the d-1 angles
    // and one levy distributed (which must lie on multiples of d)
    int idx = rng() * numSteps;
    int rnidx = idx * d;
    undo_index = idx;
    undo_values = std::vector<double>(random_numbers.begin() + rnidx, random_numbers.begin() + rnidx + d);

    random_numbers[rnidx] = std::abs(rng.cauchy(1.0));
    for(int i=1; i<d; ++i)
        random_numbers[rnidx + i] = rng();

    m_steps[idx] = genStep(random_numbers.begin() + rnidx);
    updatePoints(idx+1);

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

void LevyWalker::undoChange()
{
    int t = 0;
    for(const auto i : undo_values)
        random_numbers[undo_index * (d-1) + t++] = i;

    m_steps[undo_index] = genStep(undo_values.begin());
    updatePoints(undo_index+1);
    m_convex_hull = m_old_convex_hull;
}
