#include "RealWalker.hpp"

RealWalker::RealWalker(int d, int numSteps, const UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<double>(d, numSteps, rng_in, hull_algo, amnesia)
{
    // we need d-1 random numbers per step, for each angle one
    random_numbers = rng.vector_gaussian(d * numSteps);
    init();
}

/// Get new random numbers and reconstruct the walk
void RealWalker::reconstruct()
{
    // write new gaussian random numers into our state
    std::generate(random_numbers.begin(), random_numbers.end(),
                  [this]{ return this->rng.gaussian(); });
    init();
}

/** Generate a step by unit distance and angles determined by the
 * d-1 random numbers after first (inclusive first).
 *
 * \param first iterator to the first random number to use, must have at
 *        least $d-1$ following entries
 * \return r-value Step
 */
Step<double> RealWalker::genStep(std::vector<double>::iterator first) const
{
    Step<double> s(std::vector<double>(first, first+d));
    s *= 1./s.length();
    return s;
}

void RealWalker::updateSteps()
{
    m_steps.resize(numSteps);
    for(int i=0; i<numSteps; ++i)
        m_steps[i] = genStep(random_numbers.begin() + i*d);
}

void RealWalker::change(UniformRNG &rng, bool update)
{
    // We need d-1 random numbers per step to determine the d-1 angles
    int idx = rng() * numSteps;
    int rnidx = idx * d;
    undo_index = idx;
    undo_values = std::vector<double>(random_numbers.begin() + rnidx,
                                      random_numbers.begin() + rnidx + d);
    for(int i=0; i<d; ++i)
        random_numbers[rnidx + i] = rng();

    m_steps[idx] = genStep(random_numbers.begin() + rnidx);
    updatePoints(idx+1);

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

void RealWalker::undoChange()
{
    int t = 0;
    for(const auto i : undo_values)
        random_numbers[undo_index * d + t++] = i;

    m_steps[undo_index] = genStep(undo_values.begin());
    updatePoints(undo_index+1);
    m_convex_hull = m_old_convex_hull;
}
