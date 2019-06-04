#include "ReturningLatticeWalker.hpp"

ReturningLatticeWalker::ReturningLatticeWalker(int d, int numSteps, const UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : LatticeWalker(d, numSteps, rng_in, hull_algo, amnesia)
{
    if(numSteps % 2)
    {
        LOG(LOG_WARNING) << "Returning Walks need to have an even number of steps";
        numSteps += 1;
    }
    random_numbers = rng.vector(numSteps);
    newStep = Step<int>(d);

    permutation.resize(numSteps/2);
    for(int i=0; i<numSteps/2; ++i)
        permutation[i] = i;
    shuffle_permutation();

    init();
}

// use the second half of `random_numbers` to shuffle the permutation field
void ReturningLatticeWalker::shuffle_permutation()
{
    int offset = numSteps / 2;
    // fisher yates
    // TODO
}

void ReturningLatticeWalker::updateSteps()
{
    m_steps.clear();
    m_steps.reserve(numSteps);
    for(int i=0; i<numSteps/2; ++i)
        m_steps.emplace_back(d, random_numbers[i]);
    for(int i=0; i<numSteps/2; ++i)
        m_steps.push_back(m_steps[permutation[i]]);
}

void ReturningLatticeWalker::change(UniformRNG &rng, bool update)
{
    int idx = rng() * numSteps;
    undo_index = idx;

    if(idx < numSteps/2)
    {
        undo_value = random_numbers[idx];
        random_numbers[idx] = rng();
        newStep.fillFromRN(random_numbers[idx]);
        // test if something changes
        if(newStep == m_steps[idx])
            return;

        m_steps[idx].swap(newStep);
        updatePoints(idx+1);
    }
    else
    {
        int offset = numSteps/2;
        undo_swap = rng() * offset + offset;
        m_steps[idx].swap(m_steps[undo_swap]);
        updatePoints(std::min(undo_swap, idx)+1);
    }

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

void ReturningLatticeWalker::undoChange()
{
    if(undo_idx < numSteps/2)
    {
        random_numbers[undo_index] = undo_value;
        newStep.fillFromRN(undo_value);
    }
    else
    {
        m_steps[idx].swap(m_steps[undo_swap]);
        updatePoints(std::min(undo_swap, idx)+1);
    }

    // test if something changes
    if(newStep == m_steps[undo_index])
        return;

    m_steps[undo_index].swap(newStep);
    updatePoints(undo_index+1);
    m_convex_hull = m_old_convex_hull;
}
