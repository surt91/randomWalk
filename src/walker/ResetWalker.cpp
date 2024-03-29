#include "ResetWalker.hpp"

ResetWalker::ResetWalker(int d, int numSteps, const UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<int>(d, numSteps, rng_in, hull_algo, amnesia),
      resetrate(0.1)
{
    if(!amnesia)
        random_numbers = rng.vector(numSteps);
    init();
}

/// Get new random numbers and reconstruct the walk
void ResetWalker::reconstruct()
{
    if(!amnesia)
    {
        // write new random numers into our state
        std::generate(random_numbers.begin(), random_numbers.end(), std::ref(rng));
    }
    init();
}

void ResetWalker::updateSteps()
{
    m_steps.clear();
    m_steps.reserve(numSteps);
    Step<int> pos(d);

    m_num_resets = 0;
    m_longest_streak = 0;
    int streak = 0;
    m_longest_partial_walk = 0;
    double partial_walk = 0;

    for(int i=0; i<numSteps; ++i)
    {
        double rn;
        if(!amnesia)
            rn = random_numbers[i];
        else
            rn = rng();

        if(rn < resetrate)
        {
            // if we reset,
            // use the current position to jump to the origin and
            // use the remaining randomness for the next step
            Step<int> next(d, rn/resetrate);
            m_steps.emplace_back(-pos + next);
            ++m_num_resets;
            streak = 1;
            partial_walk = m_steps.back().length();
        }
        else
        {
            // if we do not reset, use the remaining randomness for the next step
            m_steps.emplace_back(d, rn/(1-resetrate));
            ++streak;
            partial_walk += m_steps.back().length();
        }
        pos += m_steps.back();

        if(streak > m_longest_streak)
            m_longest_streak = streak;
        if(partial_walk > m_longest_partial_walk)
            m_longest_partial_walk = partial_walk;
    }
}

/** Changes the walk, i.e., performs one trial move.
 *
 * \param rng Random number generator to draw the needed randomness from
 * \param update Should the hull be updated after the change?
 */
void ResetWalker::change(UniformRNG &rng, bool update)
{
    // I should do this in a far more clever way
    int idx = rng() * numSteps;
    undo_index = idx;
    undo_value = random_numbers[idx];
    random_numbers[idx] = rng();

    updateSteps();
    updatePoints();

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

/// Undoes the last change.
void ResetWalker::undoChange()
{
    random_numbers[undo_index] = undo_value;

    updateSteps();
    updatePoints();
    m_convex_hull = m_old_convex_hull;
}

void ResetWalker::setP1(double p1)
{
    // this is expensive, so ask first, if something changes
    if(resetrate == p1)
        return;
    resetrate = p1;
    updateSteps();
    updatePoints();
    updateHull();
}

int ResetWalker::num_resets() const
{
    if(amnesia)
        return m_num_resets;

    int num_resets = 0;

    for(int i=0; i<numSteps; ++i)
        if(random_numbers[i] < resetrate)
            ++num_resets;

    return num_resets;
}

int ResetWalker::maxsteps_partialwalk() const
{
    if(amnesia)
        return m_longest_streak;

    int longest_streak = 0;
    int streak = 0;

    for(int i=0; i<numSteps; ++i)
    {
        if(random_numbers[i] < resetrate )
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

double ResetWalker::maxlen_partialwalk() const
{
    if(amnesia)
        return m_longest_partial_walk;

    double longest_partial_walk = 0;
    int partial_walk = 0;

    for(int i=0; i<numSteps; ++i)
    {
        if(random_numbers[i] < resetrate )
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
