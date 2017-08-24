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

/** Determine the sites which are safe to visit, only d = 2
 *
 * \param current position of the head of the walk
 * \param direction the walk is pointed in, i.e., the last step
 *
 * \returns a flag of 5 bits for the directions, which ones are safe
 *          1: a left
 *          2: b straight ahead
 *          3: c right
 */
std::bitset<3> EscapeWalker::safeOptions(const Step<int> &current, const Step<int> &direction)
{
    // first: everything is safe
    std::bitset<3> safe;
    safe.set();

    auto neighbors = current.front_nneighbors(direction);
    bool a = occupied.count(neighbors[1]);
    bool b = occupied.count(neighbors[0]);
    bool c = occupied.count(neighbors[2]);
    bool d = occupied.count(neighbors[3]);
    bool e = occupied.count(neighbors[4]);

    // nothing occupied -> everything is safe
    if(!(a || b || c || d || e))
        return safe;

    // occupied sites are not safe
    safe.set(0, !a);
    safe.set(1, !b);
    safe.set(2, !c);

    // if b is occupied and both a and c not, we need to determine which of both is safe
    if(!a && b && !c)
    {
        int winding = winding_angle[occupied[current]] - winding_angle[occupied[neighbors[0]]];
        if(winding < 0)
            // left a will trap, right c is safe
            safe.set(0, false);
        if(winding > 0)
            // left a is safe, right c will trap
            safe.set(2, false);
        if(winding == 0)
        {
            LOG(LOG_WARNING) << "This should never happen!";
        }
        return safe;
    }

    // if d and e are not occupied, everything is simple
    if(!d && !e)
        return safe;

    // only d or e occupied case
    // TODO
    safe.reset();
    return safe;
}

/* test if the walk can escape to infinity, if it did the step next
 */
bool EscapeWalker::escapable(const Step<int> &next, const Step<int> &current, const Step<int> &direction, const Step<int> &next_direction)
{
    // we can not get trapped if the current step
    // only has one neighbor (but with two, we can get trapped)
    int ctr2 = 0;

    // the winding angle method for d=2
    // https://doi.org/10.1103/PhysRevLett.54.267
    // https://doi.org/10.1007/s10955-015-1271-4
    // bool test;
    if(d==2)
    {
        auto opt = safeOptions(current, direction);
        // LOG(LOG_INFO) << next_direction << " " << direction
        //               << " a: " << opt[0] << " " << next_direction.left_of(direction)
        //               << " b: " << opt[1] << " " << (next_direction == direction)
        //               << " c: " << opt[2] << " " << next_direction.right_of(direction);
        if(opt[0] && next_direction.left_of(direction))
            return true;
        if(opt[1] && next_direction == direction)
            return true;
        if(opt[2] && next_direction.right_of(direction))
            return true;
    }
    else
    {
        // higher dimensions: do a brute force search
        for(const auto &i : current.neighbors(true))
            if(occupied.count(i))
                ++ctr2;

        if(ctr2 < 2)
            return true;
    }

    // get a bounding box, such that we dont explore the whole possible lattice
    std::vector<int> min_b(d, 0);
    std::vector<int> max_b(d, 0);

    for(const auto &j : occupied)
    {
        const auto &i = j.first;
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
    bool test2 = g.bestfs(next, target, occupied);
    // if(test2 != test)
    // {
    //     LOG(LOG_WARNING) << test2 << " != " << test;
    // }
    return test2;
}

void EscapeWalker::updateSteps()
{
    int N = random_numbers.size();
    winding_angle = std::vector<int>(numSteps, 0);

    occupied.clear();
    occupied.emplace(Step<int>(d), 0);

    Step<int> head(d);

    int j = 0;
    for(int i=0; i<numSteps; ++i)
    {
        Step<int> next(d);
        Step<int> tmp(d);
        Step<int> prev(d);
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

            if(i)
                prev = m_steps[i-1];
            else
                prev = Step<int>(d);
        } while(occupied.count(tmp) || !escapable(tmp, head, prev, next));

        head += next;

        if(d==2 && i > 0)
            winding_angle[i] = m_steps[i-1].winding_angle(next) + winding_angle[i-1];

        m_steps[i] = next;
        occupied.emplace(head, i);
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
