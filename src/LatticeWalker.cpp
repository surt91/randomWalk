#include "LatticeWalker.hpp"

void LatticeWalker::change(UniformRNG &rng)
{
    steps(); // steps need to be initialized
    int idx = rng() * nRN();
    undo_index = idx;
    undo_value = random_numbers[idx];
    random_numbers[idx] = rng();

    Step<int> newStep(d, random_numbers[idx]);
    // test if something changes
    if(newStep == m_steps[idx])
        return;

    m_steps[idx] = newStep;
    points(idx+1);
    hullDirty = true;

    return;
}

void LatticeWalker::undoChange()
{
    random_numbers[undo_index] = undo_value;
    Step<int> newStep(d, undo_value);
    // test if something changes
    if(newStep == m_steps[undo_index])
        return;

    m_steps[undo_index] = newStep;
    points(undo_index+1);
    hullDirty = true;
}

/** set the random numbers such that we get an L shape
 */
void LatticeWalker::degenerateMaxVolume()
{
    for(int i=0; i<numSteps; ++i)
        random_numbers[i] = .99 / ceil((double) d * (i+1)/numSteps);

    stepsDirty = true;
    pointsDirty = true;
    hullDirty = true;
}

/** set the random numbers such that we get an L shape in d-1 dimensions
 */
void LatticeWalker::degenerateMaxSurface()
{
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99 / ceil((double) (d-1) * (i+1)/random_numbers.size());

    stepsDirty = true;
    pointsDirty = true;
    hullDirty = true;
}

/** set the random numbers such that we get a spiral
 */
void LatticeWalker::degenerateSpiral()
{
    // TODO: find some easy construction for a spiral in arbitrary dimensions
    // FIXME: right now, it is a straight line instead of a spiral
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99;

    stepsDirty = true;
    pointsDirty = true;
    hullDirty = true;
}

/** set the random numbers such that we get a straight line
 */
void LatticeWalker::degenerateStraight()
{
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99;

    stepsDirty = true;
    pointsDirty = true;
    hullDirty = true;
}
