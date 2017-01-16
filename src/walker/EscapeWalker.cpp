#include "EscapeWalker.hpp"

EscapeWalker::EscapeWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<int>(d, numSteps, rng, hull_algo, amnesia)
{
    create();
    newStep = Step<int>(d);
    init();
}

void EscapeWalker::reconstruct()
{
    create();
    init();
}

void EscapeWalker::create()
{
    random_numbers.clear();
    occupied.clear();
    occupied.insert(Step<int>(d));

    Step<int> head(d);

    for(int i=0; i<numSteps; ++i)
    {
        Step<int> next(d);
        Step<int> tmp(d);
        double rn;
        do
        {
            rn = rng();
            next.fillFromRN(rn);
            tmp = head + next;
        } while(occupied.count(tmp) || !escapable(tmp));
        head += next;
        occupied.insert(head);
        random_numbers.push_back(rn);
    }
}

/* test if the walk can escape to infinity, if it did the step next
 */
bool EscapeWalker::escapable(const Step<int> next) const
{
    // TODO: implement some efficient search, maybe A*?

    if(d != 2)
    {
        LOG(LOG_ERROR) << "this function is only implemented for d=2. Generalize it!";
        exit(1);
    }

    int min = 0;
    int max = 0;
    int t = 3;
    for(auto i : occupied)
    {
        if(min > i[0] - t)
            min = i[0] - t;
        if(min > i[1] - t)
            min = i[1] - t;
        if(max < i[0] + t)
            max = i[0] + t;
        if(max < i[1] + t)
            max = i[1] +t;
    }

    // This will be far too slow
    // But this will only be a test
    Graph g;
    int dif = max-min + 1;
    for(int i=0; i<dif; ++i)
        for(int j=0; j<dif; ++j)
        {
            Step<int> s(std::vector<int>{i+min, j+min});
            if(!occupied.count(s))
            {
                int n = i*dif+j;

                g.add_node(n);
                if(n>0)
                    g.add_edge(n, n-1);
                if(n>=dif)
                    g.add_edge(n, n-dif);
            }
        }

    int n = (next[0]-min)*dif + next[1]-min;
    return g.connected(n, 0);
}

void EscapeWalker::updateSteps()
{
    m_steps.clear();
    m_steps.reserve(numSteps);
    for(int i=0; i<numSteps; ++i)
        m_steps.emplace_back(d, random_numbers[i]);
}

bool EscapeWalker::checkOverlapFree(const std::vector<Step<int>> &l)
{
    occupied.clear();
    occupied.reserve(l.size());

    auto it(l.begin());
    while(it != l.end())
    {
        if(occupied.count(*it))
            return false;
        occupied.insert(*it);
        ++it;
    }
    return true;
}

void EscapeWalker::change(UniformRNG &rng, bool update)
{
    LOG(LOG_ERROR) << "the change method is not yet implemented in a way which is useful for Metropolis ~ no detailed balance";
    exit(1);

    int idx = rng() * nRN();
    undo_index = idx;
    undo_value = random_numbers[idx];
    undo_step = m_steps[idx];

    Step<int> head = points()[idx];
    Step<int> newStep(d);
    Step<int> tmp(d);
    do
    {
        double rn = rng();
        random_numbers[idx] = rn;
        newStep.fillFromRN(rn);
        m_steps[idx] = newStep;
        tmp = head + newStep;
        updatePoints(idx+1);
    } while(!checkOverlapFree(points()) || !escapable(tmp));

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

void EscapeWalker::undoChange()
{
    // no change happened
    if(undo_index == -1)
        return;

    random_numbers[undo_index] = undo_value;
    m_steps[undo_index] = undo_step;

    updatePoints(undo_index+1);
    m_convex_hull = m_old_convex_hull;
}
