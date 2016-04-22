#include "GaussWalker.hpp"

/** Generate a step by unit distance and angles determined by the
 * d-1 random numbers after first (inclusive first).
 *
 * Algortihm see http://en.wikipedia.org/wiki/N-sphere#Spherical_coordinates
 *
 * \param first iterator to the first random number to use, must have at
 *        least $d-1$ following entries
 * \return r-value Step
 */
Step<double> GaussWalker::genStep(std::vector<double>::iterator first) const
{
    return Step<double>(std::vector<double>(first, first+d));
}

const std::vector<Step<double>>& GaussWalker::steps() const
{
    if(!stepsDirty)
        return m_steps;

    m_steps.resize(numSteps);
    for(int i=0; i<numSteps; ++i)
        m_steps[i] = genStep(random_numbers.begin() + i*d);

    stepsDirty = false;
    return m_steps;
}

void GaussWalker::change(UniformRNG &rng)
{
    // We need d-1 random numbers per step to determine the d-1 angles
    steps(); // steps need to be initialized
    int idx = rng() * numSteps;
    int rnidx = idx * d;
    undo_index = idx;
    undo_values = std::vector<double>(random_numbers.begin() + rnidx, random_numbers.begin() + rnidx+d + 1); // +1, since the last ist exclusive
    for(int i=0; i<d-1; ++i)
        random_numbers[rnidx+i] = rng.gaussian();

    m_steps[idx] = genStep(random_numbers.begin() + rnidx);
    points(idx+1);
    hullDirty = true;

    return;
}

void GaussWalker::undoChange()
{
    int t = 0;
    for(const auto i : undo_values)
        random_numbers[undo_index*d + t++] = i;

    m_steps[undo_index] = genStep(undo_values.begin());
    points(undo_index+1);
    hullDirty = true;
}
