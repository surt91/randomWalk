#include "EscapeWalker.hpp"

EscapeWalker::EscapeWalker(int d, int numSteps, UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<int>(d, numSteps, rng_in, hull_algo, amnesia)
{
    newStep = Step<int>(d);
    undoStep = Step<int>(d);
    m_steps = std::vector<Step<int>>(numSteps);
    if(!amnesia)
        random_numbers = rng.vector(numSteps);

    init();
}

void EscapeWalker::reconstruct()
{
    if(!amnesia)
    {
        // If the random number vector is far longer than what we actually need,
        // truncate it, to save some memory bandwidth (-> computing time)
        size_t expected_space_needed = std::max(5*numSteps, 2*random_numbers_used);
        if(random_numbers.size() > expected_space_needed)
            random_numbers.resize(expected_space_needed); // resize will not free memory

        // write new random numers into our state
        std::generate(random_numbers.begin(), random_numbers.end(), std::ref(rng));
    }
    init();
}

/* test if the walk can escape to infinity, if it did the step next
 */
bool EscapeWalker::escapable(const Step<int> &next, const Step<int> &current)
{
    // TODO: implement the winding angle method (for d=2)
    // https://doi.org/10.1103/PhysRevLett.54.267

    // we can not get trapped if the current step
    // only has one neighbor (but with two, we can get trapped)
    int ctr2 = 0;
    for(const auto &i : current.neighbors(true))
        if(occupied.count(i))
            ++ctr2;
    if(ctr2 < 2)
        return true;

    // get a bounding box, such that we dont explore the whole possible lattice
    std::vector<int> min_b(d, 0);
    std::vector<int> max_b(d, 0);

    for(auto i : occupied)
    {
        for(int axis=0; axis<d; ++axis)
        {
            if(min_b[axis] > i[axis])
                min_b[axis] = i[axis];
            if(max_b[axis] < i[axis])
                max_b[axis] = i[axis];
        }
    }
    for(int axis=0; axis<d; ++axis)
    {
        min_b[axis] -= 1;
        max_b[axis] += 1;
    }

    // next find the corner of the bounding box
    // nearest to the head position such that we need only a
    // few steps to reach infinity
    std::vector<int> point(d, 0);
    Step<int> target;
    int dist = 2000000000;
    for(int i=0; i<std::pow(2, d); ++i)
    {
        for(int j=0; j<d; ++j)
        {
            point[j] = (i & (1 << j)) ? min_b[j] : max_b[j];
        }

        if(next.dist(Step<int>(point)) < dist)
        {
            target = Step<int>(point);
            dist = next.dist(target);
        }
    }
    return g.bestfs(next, target, occupied);
}

void EscapeWalker::updateSteps()
{
    int N = random_numbers.size();

    occupied.clear();
    occupied.insert(Step<int>(d));

    Step<int> head(d);

    int j = 0;
    for(int i=0; i<numSteps; ++i)
    {
        Step<int> next(d);
        Step<int> tmp(d);
        double rn;
        do
        {
            if(!amnesia)
            {
                // generate more random numbers if necessary
                if(j >= N)
                {
                    N *= 2;
                    random_numbers.resize(N);

                    std::generate(random_numbers.begin() + j, random_numbers.end(), std::ref(rng));
                }
                rn = random_numbers[j];
            }
            else
            {
                rn = rng();
            }
            ++j;

            next.fillFromRN(rn);
            tmp = head + next;
        } while(occupied.count(tmp) || !escapable(tmp, head));

        head += next;
        m_steps[i] = next;
        occupied.insert(head);
    }
    random_numbers_used = j;
}

int EscapeWalker::nRN() const
{
    return random_numbers_used;
}

void EscapeWalker::change(UniformRNG &rng, bool update)
{
    int idx = rng() * nRN();
    undo_index = idx;
    undo_value = random_numbers[idx];
    random_numbers[idx] = rng();

    newStep.fillFromRN(random_numbers[idx]);
    // test if something changes
    undoStep.fillFromRN(undo_value);
    if(newStep == undoStep)
        return;

    updateSteps();
    updatePoints();

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }

}

void EscapeWalker::undoChange()
{
    random_numbers[undo_index] = undo_value;
    // test if something changed
    if(newStep == undoStep)
        return;

    updateSteps();
    updatePoints();
    m_convex_hull = m_old_convex_hull;
}
