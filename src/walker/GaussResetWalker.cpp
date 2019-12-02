#include "GaussResetWalker.hpp"

GaussResetWalker::GaussResetWalker(int d, int numSteps, const UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<double>(d, numSteps, rng_in, hull_algo, amnesia),
      resetrate(0.0)
{
    // the random number vector has a format of one uniform rn for resetting
    // and d gaussian rn for the displacement
    random_numbers = rng.vector_gaussian((d+1) * numSteps);
    for(int i=0; i<numSteps; ++i)
        random_numbers[i*(d+1)] = rng();
    init();
}

/// Get new random numbers and reconstruct the walk
void GaussResetWalker::reconstruct()
{
    // write new random numers into our state
    std::generate(random_numbers.begin(), random_numbers.end(),
                  [this]{ return this->rng.gaussian(); });
    for(int i=0; i<numSteps; ++i)
        random_numbers[i*(d+1)] = rng();
    init();
}

void GaussResetWalker::updateSteps()
{
    m_steps.clear();
    m_steps.reserve(numSteps);
    Step<double> pos(d);
    Step<double> offset(d);
    for(int i=0; i<numSteps; ++i)
    {
        if(random_numbers[i*(d+1)] < resetrate)
        {
            offset = -pos;
        }
        else
        {
            offset.setZero();
        }

        m_steps.emplace_back(genStep(random_numbers.begin() + i*(d+1) + 1));
        m_steps.back() += offset;
        pos += m_steps.back();
    }
}

/** Changes the walk, i.e., performs one trial move.
 *
 * \param rng Random number generator to draw the needed randomness from
 * \param update Should the hull be updated after the change?
 */
void GaussResetWalker::change(UniformRNG &rng, bool update)
{
    // We need d random numbers per step to determine the d directions
    int idx = rng() * numSteps;
    int rnidx = idx * (d+1);
    undo_index = idx;
    undo_values = std::vector<double>(random_numbers.begin() + rnidx,
                                      random_numbers.begin() + rnidx + (d+1));

    // the reset rn needs to be distributed uniformly
    random_numbers[rnidx] = rng();
    // the displacement rn are distributed normally
    for(int i=1; i<d+1; ++i)
        random_numbers[rnidx+i] = rng.gaussian();

    m_steps[idx] = genStep(random_numbers.begin() + rnidx + 1);
    updatePoints(idx+1);

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

/// Undoes the last change.
void GaussResetWalker::undoChange()
{
    int t = 0;
    for(const auto i : undo_values)
        random_numbers[undo_index*(d+1) + t++] = i;

    m_steps[undo_index] = genStep(undo_values.begin());
    updatePoints(undo_index+1);
    m_convex_hull = m_old_convex_hull;
}

void GaussResetWalker::setP1(double p1)
{
    // this is expensive, so ask first, if something changes
    if(resetrate == p1)
        return;
    resetrate = p1;
    updateSteps();
    updatePoints();
    updateHull();
}

/** Generate a step by unit distance and angles determined by the
 * d-1 random numbers after first (inclusive first).
 *
 * Algortihm see http://en.wikipedia.org/wiki/N-sphere#Spherical_coordinates
 *
 * \param first iterator to the first random number to use, must have at
 *        least $d-1$ following entries
 * \return r-value Step
 */
Step<double> GaussResetWalker::genStep(std::vector<double>::iterator first) const
{
    return Step<double>(std::vector<double>(first, first+d));
}
