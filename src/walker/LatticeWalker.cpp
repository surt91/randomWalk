#include "LatticeWalker.hpp"

void LatticeWalker::updateSteps()
{
    m_steps.clear();
    m_steps.reserve(numSteps);
    for(int i=0; i<numSteps; ++i)
        m_steps.emplace_back(d, random_numbers[i]);
}

void LatticeWalker::change(UniformRNG &rng)
{
    int idx = rng() * nRN();
    undo_index = idx;
    undo_value = random_numbers[idx];
    random_numbers[idx] = rng();

    Step<int> newStep(d, random_numbers[idx]);
    // test if something changes
    if(newStep == m_steps[idx])
        return;

    m_steps[idx] = newStep;
    updatePoints(idx+1);
    updateHull();
}

void LatticeWalker::undoChange()
{
    random_numbers[undo_index] = undo_value;
    Step<int> newStep(d, undo_value);
    // test if something changes
    if(newStep == m_steps[undo_index])
        return;

    m_steps[undo_index] = newStep;
    updatePoints(undo_index+1);
    updateHull();
}
