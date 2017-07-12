#include "ScentWalker.hpp"

// TODO this does not seem very abstract -> rename to ScentWalker

ScentWalker::ScentWalker(int d, int numSteps, int numWalker_in, UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker(d, numSteps, rng, hull_algo, amnesia),
      numWalker(numWalker_in)
{
    random_numbers = rng.vector(numSteps*numWalker);
    init();
}

void ScentWalker::updateSteps()
{
    // TODO
    // start the agent simulation
    // iterate the time, every agent does one move each timestep
    // use numWalker vectors for the steps (scent traces will be steps[now-Tas:now])
    // every walker has its own history

    // data structures: hashmap/bitmap: site -> deque[(time of last visit, who visited)]
    //  at every visit remove expired entries from the back of the deque
    //  and entries of oneself (because oneself left a new scent in that moment)

    // also, we need periodic boundaries and a fixed size on this one

    // populate the histogram (for a figure as in the artikel)

    // maybe update the points inside this function
    // if they are always up to date, `updatePoints` can be a noop

    // copy steps and points of walker 0 to m_steps and m_points
    // and everything will work -- though with a bit of overhead
}

void ScentWalker::updatePoints(const int /*start*/)
{
    // points are always up to date, see `ScentWalker::updateSteps()`
}

void ScentWalker::change(UniformRNG &rng, bool update)
{
    LOG(LOG_WARNING) << "not yet implemented";
}

void ScentWalker::undoChange()
{
    LOG(LOG_WARNING) << "not yet implemented";
}
