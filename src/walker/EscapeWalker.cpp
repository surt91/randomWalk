#include "EscapeWalker.hpp"

EscapeWalker::EscapeWalker(int d, int numSteps, UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<int>(d, numSteps, rng_in, hull_algo, amnesia)
{
    newStep = Step<int>(d);
    undoStep = Step<int>(d);
    winding_angle = std::vector<int>(numSteps, 0);
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
 * Uses the winding angle method for d=2:
 * https://doi.org/10.1103/PhysRevLett.54.267
 * https://doi.org/10.1007/s10955-015-1271-4
 *
 * \param current position of the head of the walk
 * \param direction the walk is pointed in, i.e., the last step
 *
 * \returns a flag of 5 bits for the directions, which ones are safe
 *          1: a left
 *          2: b straight ahead
 *          3: c right
 *
 * \image html safeEscape.svg "names of the sites"
 */
std::bitset<3> EscapeWalker::safeOptions(const Step<int> &current, const Step<int> &direction)
{
    // first: everything is safe
    std::bitset<3> safe;
    safe.set();

    // if direction is zero -> this happens at the first step -> everything is safe
    if(direction.length2() == 0)
        return safe;

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

    if(a && b && c)
    {
        LOG(LOG_ERROR) << "I am trapped!";
        LOG(LOG_INFO) << "from " << current << " look at" << neighbors;
        LOG(LOG_INFO) << b << " " << a << " " << c << " " << d << " " << e << " ";
        exit(1);
    }

    // if there is only one option, take it, it will never trap
    if((a && b) || (b && c) || (a && c))
        return safe;

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

    // if d and e are both occupied
    if(d && e)
    {
        int w = winding_angle[occupied[current]];
        const int w_d = winding_angle[occupied[neighbors[3]]];
        const int w_e = winding_angle[occupied[neighbors[4]]];

        if(w > w_d && w > w_e)
        {
            safe.set(2, false);
            safe.set(1, false);
        }
        else if(w < w_d && w < w_e)
        {
            safe.set(0, false);
            safe.set(1, false);
        }
        else if((w < w_d && w_e < w) || (w < w_e && w_d < w))
        {
            safe.set(0, false);
            safe.set(2, false);
        }
        else
        {
            LOG(LOG_WARNING) << "This should never happen!";
            safe.reset();
        }

        return safe;
    }

    // only d or e occupied case (last remaining option for non-trapped walks)
    if(d)
    {
        int winding = winding_angle[occupied[current]] - winding_angle[occupied[neighbors[3]]];
        if(winding < 0 || a)
        {
            // left a will trap, c and b are safe
            safe.set(0, false);
        }
        if(winding > 0 || c)
        {
            // left a is safe, c and b will trap
            safe.set(2, false);
            safe.set(1, false);
        }
        if(winding == 0)
        {
            if(!a && !c)
            {
                LOG(LOG_WARNING) << "This should never happen!";
            }
        }
        return safe;
    }
    if(e)
    {
        int winding = winding_angle[occupied[current]] - winding_angle[occupied[neighbors[4]]];
        if(winding < 0 || a)
        {
            // left a and b will trap, c is safe
            safe.set(0, false);
            safe.set(1, false);
        }
        if(winding > 0 || c)
        {
            // left a and b are safe, c will trap
            safe.set(2, false);
        }
        if(winding == 0)
        {
            if(!a && !c)
            {
                LOG(LOG_WARNING) << "This should never happen!";
            }
        }
        return safe;
    }

    LOG(LOG_WARNING) << "This should never happen!";
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

    if(d==2)
    {
        // in d = 2 use winding angle method
        auto opt = safeOptions(current, direction);
        if(opt[0] && next_direction.left_of(direction))
            return true;
        if(opt[1] && next_direction == direction)
            return true;
        if(opt[2] && next_direction.right_of(direction))
            return true;
    }
    else
    {
        // higher dimensions: do a brute force search, if it is possible that
        // we get trapped
        for(const auto &i : current.neighbors(true))
            if(occupied.count(i))
                ++ctr2;

        // in d = 3 wee need to pass through a compact ring of 4 occupied sites
        // d > 3 needs even more occupied sites
        // this will only happen very rarely -> no need to optimize it
        if(ctr2 < 3) // there are instances where i get trapped with < 4 // TODO: investigate
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
    return g.bestfs(next, target, occupied);
}

void EscapeWalker::updateSteps()
{
    occupied.clear();
    occupied.emplace(Step<int>(d), 0);

    Step<int> head(d);
    Step<int> next(d);
    Step<int> tmp(d);
    Xorshift xorshift(0);

    for(int i=0; i<numSteps; ++i)
    {
        Step<int> prev(d);

        if(i)
            prev = m_steps[i-1];
        else
            prev = Step<int>(d);

        double rn;
        if(!amnesia)
            rn = random_numbers[i];
        else
            rn = rng();

        next.fillFromRN(rn);
        double next_rn = rn;

        // TODO: get a array of allowed steps and choose one
        while(true)
        {
            tmp = head + next;

            if(occupied.count(tmp) || !escapable(tmp, head, prev, next))
            {
                // if the wanted site is not available, we will test
                // another site in a deterministic fashion

                // only seed the generator if we need it
                if(next_rn == rn) {
                    // use as much entropy from the double as possible
                    xorshift.seed((uint64_t) (rn * std::pow(2, 52)));
                }

                do {
                    next_rn += xorshift();
                    next_rn -= floor(next_rn);
                    next.fillFromRN(next_rn);
                } while(next == -prev); // do not step back
            }
            else
                break;
        }

        head += next;

        if(d==2 && i > 0)
            winding_angle[i] = m_steps[i-1].winding_angle(next) + winding_angle[i-1];

        m_steps[i] = next;
        occupied.emplace(head, i);
    }
}

void EscapeWalker::change(UniformRNG &rng, bool update)
{
    // we need local changes, e.g., crankshaft
    // the global changes used in other walks, will result in many
    // new random numbers after the change, since the self avoing property
    // will force new turns -- this way equilibration is not possible

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
