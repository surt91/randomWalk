#include "TrueSelfAvoidingWalker.hpp"

TrueSelfAvoidingWalker::TrueSelfAvoidingWalker(int d, int numSteps, const UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<int>(d, numSteps, rng_in, hull_algo, amnesia),
      beta(1.0)
{
    newStep = Step<int>(d);
    undoStep = Step<int>(d);
    if(!amnesia)
        random_numbers = rng.vector(numSteps);
    init();
}

/// Get new random numbers and reconstruct the walk
void TrueSelfAvoidingWalker::reconstruct()
{
    if(!amnesia)
    {
        // write new random numers into our state
        std::generate(random_numbers.begin(), random_numbers.end(), std::ref(rng));
    }
    init();
}

void TrueSelfAvoidingWalker::updateSteps()
{
    m_steps.clear();
    m_steps.reserve(numSteps);

    std::unordered_map<Step<int>, int> number_of_visits;

    // build the whole walk here

    // probabilities to step on the neighboring sites
    std::vector<double> p(2*d);
    // current position
    Step<int> head(d);
    if(!amnesia)
        head.fillFromRN(random_numbers[0]);
    else
        head.fillFromRN(rng());

    m_steps.push_back(head);
    for(int t=1; t<numSteps; ++t)
    {
        // iterate over neighbors, to update the probabilites p
        int ctr = 0;
        double norm = 0.;
        std::vector<Step<int>> neighbors = head.neighbors();
        ++number_of_visits[head];
        for(const auto &i : neighbors)
        {
            int times_visited = number_of_visits[i];
            double prob = std::exp(-beta*times_visited);
            norm += prob;
            p[ctr] = norm; // p is cumulative probability function
            ++ctr;
        }

        // step on a neighbor according to p
        double rn = norm;
        if(!amnesia)
            rn *= random_numbers[t];
        else
            rn *= rng();

        int idx = 0;
        // for high dimensions a bisection would make sense, but for low not
        while(rn > p[idx])
        {
            ++idx;
        }
        m_steps.emplace_back(neighbors[idx] - head);
        head = neighbors[idx];
    }
}

/** Changes the walk, i.e., performs one trial move.
 *
 * \param rng Random number generator to draw the needed randomness from
 * \param update Should the hull be updated after the change?
 */
void TrueSelfAvoidingWalker::change(UniformRNG &rng, bool update)
{
    // I should do this in a far more clever way
    int idx = rng() * numSteps;
    undo_index = idx;
    undo_value = random_numbers[idx];
    random_numbers[idx] = rng();

    updateSteps();
    updatePoints();

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

/// Undoes the last change.
void TrueSelfAvoidingWalker::undoChange()
{
    random_numbers[undo_index] = undo_value;

    updateSteps();
    updatePoints();
    m_convex_hull = m_old_convex_hull;
}

void TrueSelfAvoidingWalker::setP1(double p1)
{
    // this is expensive, so ask first, if something changes
    if(beta == p1)
        return;
    beta = p1;
    updateSteps();
    updatePoints();
    updateHull();
}
