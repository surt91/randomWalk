#include "BrownianResetWalker.hpp"

BrownianResetWalker::BrownianResetWalker(int d, int numSteps, const UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<double>(d, numSteps, rng_in, hull_algo, amnesia),
      resetrate(0.0),
      total_time(numSteps),
      delta_t(1.0)
{
    // the random number vector has a format of one uniform rn for resetting
    // and d gaussian rn for the displacement
    random_numbers = rng.vector_gaussian((d+1) * numSteps);
    for(int i=0; i<numSteps; ++i)
        random_numbers[i*(d+1)] = rng();
    init();
}

/// Get new random numbers and reconstruct the walk
void BrownianResetWalker::reconstruct()
{
    // write new random numers into our state
    std::generate(random_numbers.begin(), random_numbers.end(),
                  [this]{ return this->rng.gaussian(); });
    for(int i=0; i<numSteps; ++i)
        random_numbers[i*(d+1)] = rng();
    init();
}

void BrownianResetWalker::updateSteps()
{
    m_steps.clear();
    m_steps.reserve(numSteps);
    Step<double> pos(d);
    Step<double> offset(d);
    const double sdt = sqrt(delta_t);

    for(int i=0; i<numSteps; ++i)
    {
        if(random_numbers[i*(d+1)] < resetrate * delta_t)
        {
            offset = -pos;
        }
        else
        {
            offset.setZero();
        }

        m_steps.emplace_back(genStep(random_numbers.begin() + i*(d+1) + 1));
        m_steps.back() *= sdt;
        m_steps.back() += offset;
        pos += m_steps.back();
    }
}

/** Changes the walk, i.e., performs one trial move.
 *
 * \param rng Random number generator to draw the needed randomness from
 * \param update Should the hull be updated after the change?
 */
void BrownianResetWalker::change(UniformRNG &rng, bool update)
{
    // We need d random numbers per step to determine the d directions
    int idx = rng() * numSteps;
    int rnidx = idx * (d+1);
    undo_index = idx;
    undo_values = std::vector<double>(random_numbers.begin() + rnidx,
                                      random_numbers.begin() + rnidx + (d+1));

    if(rng() > 0.5)
    {
        // the reset rn needs to be distributed uniformly
        random_numbers[rnidx] = rng();
    }
    else
    {
        // the displacement rn are distributed normally
        for(int i=1; i<d+1; ++i)
            random_numbers[rnidx+i] = rng.gaussian();
    }

    m_steps[idx] = genStep(random_numbers.begin() + rnidx + 1);
    m_steps[idx] *= sqrt(delta_t);
    updatePoints(idx+1);

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

/// Undoes the last change.
void BrownianResetWalker::undoChange()
{
    int t = 0;
    for(const auto i : undo_values)
        random_numbers[undo_index*(d+1) + t++] = i;

    m_steps[undo_index] = genStep(undo_values.begin() + 1);
    m_steps[undo_index] *= sqrt(delta_t);
    updatePoints(undo_index+1);
    m_convex_hull = m_old_convex_hull;
}

void BrownianResetWalker::setP1(double p1)
{
    // this is expensive, so ask first, if something changes
    if(resetrate == p1)
        return;
    resetrate = p1;
    updateSteps();
    updatePoints();
    updateHull();
}

void BrownianResetWalker::setP2(double p2)
{
    // this is expensive, so ask first, if something changes
    if(total_time == p2)
        return;
    total_time = p2;
    delta_t = total_time / numSteps;
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
Step<double> BrownianResetWalker::genStep(std::vector<double>::iterator first) const
{
    return Step<double>(std::vector<double>(first, first+d));
}

double BrownianResetWalker::argminx() const
{
    return SpecWalker<double>::argminx() * delta_t;
}

double BrownianResetWalker::argmaxx() const
{
    return SpecWalker<double>::argmaxx() * delta_t;
}

int BrownianResetWalker::num_resets() const
{
    int num_resets = 0;

    for(int i=0; i<numSteps; ++i)
        if(random_numbers[i*(d+1)] < resetrate * delta_t)
            ++num_resets;

    return num_resets;
}

int BrownianResetWalker::maxsteps_partialwalk() const
{
    int longest_streak = 0;
    int streak = 0;

    for(int i=0; i<numSteps; ++i)
    {
        if(random_numbers[i*(d+1)] < resetrate * delta_t)
        {
            streak = 1;
        }
        else
        {
            ++streak;
        }

        if(streak > longest_streak)
            longest_streak = streak;
    }

    return longest_streak;
}


double BrownianResetWalker::maxlen_partialwalk() const
{
    double longest_partial_walk = 0;
    double partial_walk = 0;

    for(int i=0; i<numSteps; ++i)
    {
        if(random_numbers[i*(d+1)] < resetrate )
        {
            partial_walk = m_steps[i].length();
        }
        else
        {
            partial_walk += m_steps[i].length();
        }

        if(partial_walk > longest_partial_walk)
            longest_partial_walk = partial_walk;
    }

    return longest_partial_walk;
}
