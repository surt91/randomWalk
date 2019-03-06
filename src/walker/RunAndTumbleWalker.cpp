#include "RunAndTumbleWalker.hpp"

// exact mean perimeter: g = 0.5; f(x) = 1/g*(-(pi+2)+2*sqrt(pi)*(gamma(2.+floor((n-1)/2.))/gamma(3./2.+floor(n-1)/2.) + gamma(3./2. + floor(n/2.))/gamma(1.+floor(n/2.)))
RunAndTumbleWalker::RunAndTumbleWalker(int d, int numSteps, const UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<double>(d, numSteps, rng_in, hull_algo, amnesia),
      gamma(0.5)
{
    // we need d gaussian random numbers per step, for each direction
    random_numbers = rng.vector_gaussian(d * numSteps);
    // we also need one random boolean per step to decide if the direction is changed
    random_tumble = rng.vector(numSteps);
    init();
}

/// Get new random numbers and reconstruct the walk
void RunAndTumbleWalker::reconstruct()
{
    // write new gaussian random numers into our state
    std::generate(random_numbers.begin(), random_numbers.end(),
                  [this]{ return this->rng.gaussian(); });

    std::generate(random_tumble.begin(), random_tumble.end(),
                  [this]{ return this->rng.uniform(); });

    init();
}

void RunAndTumbleWalker::setP1(double gamma_in)
{
    gamma = gamma_in;
}

/** Generate a step
 *
 * \param idx index of the step to be generated, this walk has a kind of memory
            and needs sometimes the direction of the preceding step
 * \return r-value Step
 */
Step<double> RunAndTumbleWalker::genStep(int idx) const
{
    double r = -std::log(random_tumble[idx])/gamma;
    auto first = random_numbers.begin() + idx*d;
    Step<double> s(std::vector<double>(first, first+d));
    s *= r/s.length();

    return s;
}

void RunAndTumbleWalker::updateSteps()
{
    m_steps.resize(numSteps);
    for(int i=0; i<numSteps; ++i)
        m_steps[i] = genStep(i);
}

void RunAndTumbleWalker::change(UniformRNG &rng, bool update)
{
    // We need d random numbers per step to determine the d directions
    steps(); // steps need to be initialized
    int idx = rng() * numSteps;
    int rnidx = idx * d;
    undo_index = idx;
    undo_values = std::vector<double>(
                    random_numbers.begin() + rnidx,
                    random_numbers.begin() + rnidx + d
                );
    undo_tumble = random_tumble[idx];

    // change the tumble random number or step random number half of the time
    if(rng() > 0.5)
    {
        for(int i=0; i<d; ++i)
            random_numbers[rnidx+i] = rng.gaussian();
    }
    else
    {
        random_tumble[idx] = rng();
    }

    m_steps[idx] = genStep(idx);
    updatePoints(idx+1);

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

void RunAndTumbleWalker::undoChange()
{
    int t = 0;
    for(const auto i : undo_values)
        random_numbers[undo_index*d + t++] = i;
    random_tumble[undo_index] = undo_tumble;

    m_steps[undo_index] = genStep(undo_index);
    updatePoints(undo_index+1);
    m_convex_hull = m_old_convex_hull;
}
