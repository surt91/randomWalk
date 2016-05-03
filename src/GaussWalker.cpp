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

void GaussWalker::updateSteps()
{
    m_steps.resize(numSteps);
    for(int i=0; i<numSteps; ++i)
        m_steps[i] = genStep(random_numbers.begin() + i*d);
}

void GaussWalker::change(UniformRNG &rng)
{
    // We need d random numbers per step to determine the d directions
    steps(); // steps need to be initialized
    int idx = rng() * numSteps;
    int rnidx = idx * d;
    undo_index = idx;
    undo_values = std::vector<double>(random_numbers.begin() + rnidx, random_numbers.begin() + rnidx + d);
    for(int i=0; i<d; ++i)
        random_numbers[rnidx+i] = rng.gaussian();

    m_steps[idx] = genStep(random_numbers.begin() + rnidx);
    updatePoints(idx+1);
    updateHull();
}

void GaussWalker::undoChange()
{
    int t = 0;
    for(const auto i : undo_values)
        random_numbers[undo_index*d + t++] = i;

    m_steps[undo_index] = genStep(undo_values.begin());
    updatePoints(undo_index+1);
    updateHull();
}

/** Change only one component of the gauss steps.
 *
 * This seems to not offer great advantages
 */
void GaussWalker::changeSingle(UniformRNG &rng)
{
    // We need d random numbers per step to determine the d directions
    steps(); // steps need to be initialized
    int rnidx = rng() * numSteps * d;
    int idx = rnidx / d;
    undo_index = rnidx;
    undo_value = random_numbers[rnidx];
    random_numbers[rnidx] = rng.gaussian();

    m_steps[idx] = genStep(random_numbers.begin() + idx*d);
    updatePoints(idx+1);
    updateHull();
}

/** Undo the single component change.
 */
void GaussWalker::undoChangeSingle()
{
    random_numbers[undo_index] = undo_value;
    int idx = undo_index / d;

    m_steps[idx] = genStep(random_numbers.begin() + idx*d);
    updatePoints(idx+1);
    updateHull();
}

/** Set the random numbers such that we get an half circle shape.
 *
 * The distance will be chosen as 5, which should correspond to something
 * at least as rare as a 5 sigma event (in fact far, far rarer ),
 * i.e. bigger than most gaussian walks.
 */
void GaussWalker::degenerateMaxVolume()
{
    // FIXME: works only for d=2
    // I am not sure how the greatest volume in d=3 is constructed
    double r = 2;
    for(int i=0; i<numSteps; ++i)
    {
        double theta = M_PI / (i+1);
        random_numbers[i*d] = r * sin(theta);
        random_numbers[i*d+1] = r * cos(theta);
        for(int j=2; j<d; ++j)
            random_numbers[i*d+j] = i < numSteps/2 ? 0 : r;
    }

    updateSteps();
    updatePoints();
    updateHull();
}
