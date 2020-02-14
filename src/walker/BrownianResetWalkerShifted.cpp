#include "BrownianResetWalkerShifted.hpp"

BrownianResetWalkerShifted::BrownianResetWalkerShifted(int d, int numSteps, const UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : BrownianResetWalker(d, numSteps, rng_in, hull_algo, amnesia),
      shift(0.0)
{
    // the random number vector has a format of one uniform rn for resetting
    // and d gaussian rn for the displacement
    random_numbers = rng.vector_gaussian((d+1) * numSteps);
    for(int i=0; i<numSteps; ++i)
        random_numbers[i*(d+1)] = rng();
    init();
}

void BrownianResetWalkerShifted::updateSteps()
{
    m_steps.clear();
    m_steps.reserve(numSteps);
    Step<double> pos(d);
    Step<double> offset(d);
    const double sdt = sqrt(delta_t);
    double segment_time = 0.;

    for(int i=0; i<numSteps; ++i)
    {
        segment_time += delta_t;
        if(random_numbers[i*(d+1)] < resetrate * delta_t && segment_time > shift)
        {
            offset = -pos;
            segment_time = 0.;
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

void BrownianResetWalkerShifted::setP3(double p3)
{
    // this is expensive, so ask first, if something changes
    if(shift == p3)
        return;
    shift = p3;
    updateSteps();
    updatePoints();
    updateHull();
}
