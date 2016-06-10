#include "LatticeWalker.hpp"

LatticeWalker::LatticeWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
    : SpecWalker<int>(d, numSteps, rng, hull_algo)
{
    random_numbers = rng.vector(numSteps);
    newStep = Step<int>(d);
    init();
}

void LatticeWalker::updateSteps()
{
    m_steps.clear();
    m_steps.reserve(numSteps);
    for(int i=0; i<numSteps; ++i)
        m_steps.emplace_back(d, random_numbers[i]);
}

void LatticeWalker::change(UniformRNG &rng, bool update)
{
    int idx = rng() * nRN();
    undo_index = idx;
    undo_value = random_numbers[idx];
    random_numbers[idx] = rng();

    newStep.fillFromRN(random_numbers[idx]);
    // test if something changes
    if(newStep == m_steps[idx])
        return;

    m_steps[idx].swap(newStep);
    updatePoints(idx+1);

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

void LatticeWalker::undoChange()
{
    random_numbers[undo_index] = undo_value;
    newStep.fillFromRN(undo_value);
    // test if something changes
    if(newStep == m_steps[undo_index])
        return;

    m_steps[undo_index].swap(newStep);
    updatePoints(undo_index+1);
    m_convex_hull = m_old_convex_hull;
}
