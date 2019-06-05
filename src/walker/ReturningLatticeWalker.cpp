#include "ReturningLatticeWalker.hpp"

ReturningLatticeWalker::ReturningLatticeWalker(int d, int numSteps, const UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<int>(d, numSteps, rng_in, hull_algo, amnesia)
{
    if(numSteps % 2)
    {
        LOG(LOG_ERROR) << "Returning Walks need to have an even number of steps";
        exit(1);
    }
    random_numbers = rng.vector(numSteps);
    newStep = Step<int>(d);

    permutation = Permutation(numSteps / 2);
    permutation.sort();
    permutation.shuffle(random_numbers.begin(), random_numbers.end());

    init();
}

void ReturningLatticeWalker::reconstruct()
{
    // write new random numers into our state
    std::generate(random_numbers.begin(), random_numbers.end(), std::ref(rng));

    permutation.sort();
    permutation.shuffle(random_numbers.begin(), random_numbers.end());

    init();
}

void ReturningLatticeWalker::updateSteps()
{
    m_steps.clear();
    m_steps.reserve(numSteps);
    for(int i=0; i<numSteps/2; ++i)
        m_steps.emplace_back(d, random_numbers[i]);
    for(int i=0; i<numSteps/2; ++i)
        m_steps.push_back(-m_steps[permutation[i]]);
}

void ReturningLatticeWalker::change(UniformRNG &rng, bool update)
{
    int idx = rng() * numSteps;
    int offset = numSteps / 2;
    undo_index = idx;

    if(idx < offset)
    {
        undo_value = random_numbers[idx];
        random_numbers[idx] = rng();
        newStep.fillFromRN(random_numbers[idx]);
        // test if something changes
        if(newStep == m_steps[idx])
            return;

        m_steps[permutation.inverse(idx)+offset] = -Step<int>(d, newStep.readToRN());
        m_steps[idx].swap(newStep);

        updatePoints(idx+1);
    }
    else
    {
        undo_swap = rng() * offset + offset;

        permutation.swap(idx-offset, undo_swap-offset);
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
    int offset = numSteps / 2;
    if(undo_index < offset)
    {
        random_numbers[undo_index] = undo_value;
        newStep.fillFromRN(undo_value);
        // test if something changes
        if(newStep == m_steps[undo_index])
            return;

        m_steps[permutation.inverse(undo_index)+offset] = -Step<int>(d, undo_value);
        m_steps[undo_index].swap(newStep);

    }
    else
    {
        permutation.swap(undo_index-offset, undo_swap-offset);
        m_steps[undo_index].swap(m_steps[undo_swap]);
        updatePoints(std::min(undo_swap, undo_index)+1);
    }

    updatePoints(undo_index+1);
    m_convex_hull = m_old_convex_hull;
}
