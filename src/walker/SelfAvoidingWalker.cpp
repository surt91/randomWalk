#include "SelfAvoidingWalker.hpp"

// Possibilities:
//  naive approach
//      walk randomly, if we step on ourself, discard everything and start again
//      + easy to apply large deviation Scheme
//      - takes forever
//
//  dimerization
//      (Madras2013 The Self-Avoiding Walk)
//      divide and conquer, merge two subwalks, that may not overlap
//      + should be easy to apply large deviation scheme
//      + could be used as a starting point for pivoting or local modification
//      - also takes forever
//      + but slightly less forever
//
//  slithering snake
//      remove one step from the beginning and append randomly at the end
//      + easy
//      - not ergodic .. needs to be combined with some other change?
//      - not much faster than simple changes, probably
//
//  pivot algorithm
//      generate somehow one SAW (dimerization), choose a pivot and
//      turn everything behind it clock-, or counterclockwise
//      + generates many SAW instances
//      + could be used as the "change" step of the large deviation scheme
//      - could introduce too much change
//
//  pruned enriched Rosenbluth method (PERM)
//      grow SAWs with weights, create a population of SAWs
//      by duplicating or killing them based on their weight
//      if a walk reaches the length of N, take it as a sample
//      and continue with the next in the population
//      + generates many SAW instances, quite efficiently
//      - where can I plug in the large deviation "bias"? In the weights? How?

/// transformation matrices for pivoting d=2
static const int tMatrix2[][4] =
{
    // mirror at x-axis
        { 1,  0,
          0, -1},
    // mirror at y-axis
        {-1,  0,
          0,  1},
    // rotate by pi/2
        { 0,  1,
         -1,  0},
    // rotate by -pi/2
        { 0, -1,
          1,  0}
};
/// inverse transformations
static const int iMatrix2[] =
{
    0,
    1,
    3,
    2
};

/// transformation matrices for pivoting d=3
static const int tMatrix3[][9] =
{
    // mirror at xy-plane
        { 1,  0,  0,
          0,  1,  0,
          0,  0, -1},
    // mirror at zy-plane
        {-1,  0,  0,
          0,  1,  0,
          0,  0,  1},
    // mirror at xz-plane
        { 1,  0,  0,
          0, -1,  0,
          0,  0,  1},
    // rotate by pi/2 around x-axis
        { 1,  0,  0,
          0,  0,  1,
          0, -1,  0},
    // rotate by pi around x-axis
        { 1,  0,  0,
          0, -1,  0,
          0,  0, -1},
    // rotate by -pi/2 around x-axis
        { 1,  0,  0,
          0,  0, -1,
          0,  1,  0},
    // rotate by pi/2 around y-axis
        { 0,  0,  1,
          0,  1,  0,
         -1,  0,  0},
    // rotate by pi around y-axis
        {-1,  0,  0,
          0,  1,  0,
          0,  0, -1},
    // rotate by -pi/2 around y-axis
        { 0,  0, -1,
          0,  1,  0,
          1,  0,  0},
    // rotate by pi/2 around z-axis
        { 0, -1,  0,
          1,  0,  0,
          0,  0,  1},
    // rotate by pi around z-axis
        {-1,  0,  0,
          0, -1,  0,
          0,  0,  1},
    // rotate by -pi/2 around z-axis
        { 0,  1,  0,
         -1,  0,  0,
          0,  0,  1}
};
/// inverse transformations
static const int iMatrix3[] =
{
    0,
    1,
    2,
    5,
    4,
    3,
    8,
    7,
    6,
    11,
    10,
    9
};

SelfAvoidingWalker::SelfAvoidingWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
    : SpecWalker<int>(d, numSteps, rng, hull_algo)
{
    auto l(dim(numSteps));
    random_numbers = std::vector<double>(l.begin(), l.end());
    init();
}

void SelfAvoidingWalker::updateSteps()
{
    m_steps.clear();
    m_steps.reserve(numSteps);
    for(int i=0; i<numSteps; ++i)
        m_steps.emplace_back(d, random_numbers[i]);
}

// TODO: this version does not equilibrate for theta > -50
//       further kind of change? Parallel Tempering?
void SelfAvoidingWalker::change(UniformRNG &rng)
{
    // do a pivot change
    // choose the pivot
    int idx = rng() * nRN();
    undo_index = idx;
    int symmetry;
    switch(d)
    {
        case 2:
            symmetry = rng() * 4; // integer between 0 and 3
            undo_symmetry = iMatrix2[symmetry];
            break;
        case 3:
            symmetry = rng() * 12; // integer between 0 and 11
            undo_symmetry = iMatrix3[symmetry];
            break;
        default:
            throw std::invalid_argument("Pivot algorithm only implemented for d=2 and d=3");
    }

    // choose the change algorithm randomly
    // 20% pivot chance, 80% naive change
    if(rng() > 0.8)
    {
        pivot(idx, symmetry);
    }
    else
    {
        undo_index = -1;
        int idx = rng() * nRN();
        auto val = rng();
        naiveChange(idx, val);
    }
}

void SelfAvoidingWalker::undoChange()
{
    // which change was done
    if(undo_index == -1)
        naiveChangeUndo();
    else
        pivot(undo_index, undo_symmetry);
}

Step<int> SelfAvoidingWalker::transform(Step<int> &p, const int *m) const
{
    Step<int> out(d);
    for(int i=0; i<d; ++i)
        for(int j=0; j<d; ++j)
            out[i] += p[j] * m[i*d + j];

    return out;
}

bool SelfAvoidingWalker::pivot(const int index, const int op)
{
    const int* matrix;
    // FIXME: implement for d > 3
    if(d > 3)
        throw std::invalid_argument("Pivot algorithm only implemented for d<=3");

    // choose the symmetry operation
    switch(d)
    {
        case 2:
            matrix = tMatrix2[op];
            break;
        case 3:
            matrix = tMatrix3[op];
            break;
        default:
            throw std::invalid_argument("Pivot algorithm only implemented for d<=3");
    }

    // FIXME: pivot the shorter end
    // first just a dry test -- most of the time it will fail and it is
    // probably cheaper to do it twice in case of success instead of
    // undo much work everytime
    std::unordered_set<Step<int>> overlap_test;
    overlap_test.reserve(numSteps);
    Step<int> positioni(d);
    Step<int> positionj(d);
    overlap_test.emplace(d);
    bool failed = false;
    // test for overlaps starting at the pivot point in both directions
    for(int i=index-1, j=index; i>=0 || j<numSteps; --i, ++j)
    {
        if(i >= 0)
        {
            positioni -= m_steps[i];
            if(overlap_test.count(positioni))
            {
                failed = true;
                break;
            }
            overlap_test.insert(positioni);
        }

        if(j < numSteps)
        {
            positionj += transform(m_steps[j], matrix);
            if(overlap_test.count(positionj))
            {
                failed = true;
                break;
            }
            overlap_test.insert(positionj);
        }
    }
    if(!failed)
    {
        for(int i=index; i<numSteps; ++i)
            m_steps[i] = transform(m_steps[i], matrix);

        updatePoints(index+1);
        updateHull();
    }

    return !failed;
}

void SelfAvoidingWalker::naiveChangeUndo()
{
    // no change happened
    if(undo_naive_index == -1)
        return;

    m_steps[undo_naive_index] = undo_naive_step;

    updatePoints(undo_naive_index+1);
    updateHull();
}

bool SelfAvoidingWalker::naiveChange(const int idx, const double rn)
{
    undo_naive_index = idx;
    undo_naive_step = m_steps[idx];

    Step<int> newStep(d, rn);
    // test if something changes
    if(newStep == m_steps[idx])
        return true;

    m_steps[idx] = newStep;
    updatePoints(idx+1);

    if(!checkOverlapFree(points()))
    {
        m_steps[idx] = undo_naive_step;
        updatePoints(idx+1);
        undo_naive_index = -1;
        return false;
    }

    updateHull();

    return true;
}

//~ bool SelfAvoidingWalker::slitheringSnake(const int front, const double rn)
//~ {
    //~ if(front)
//~ }

bool SelfAvoidingWalker::checkOverlapFree(const std::vector<Step<int>> &l) const
{
    std::unordered_set<Step<int>> map;
    map.reserve(l.size());

    auto it(l.begin());
    while(it != l.end())
    {
        if(map.count(*it))
            return false;
        map.insert(*it);
        ++it;
    }
    return true;
}

bool SelfAvoidingWalker::checkOverlapFree(const std::list<double> &l) const
{
    std::unordered_set<Step<int>> map;
    map.reserve(l.size());

    auto it(l.begin());
    Step<int> p(d);
    while(it != l.end())
    {
        p += Step<int>(d, *it);
        if(map.count(p))
            return false;
        map.insert(p);
        ++it;
    }
    return true;
}

/** Dimerization algorithm to construct a Self-Avoiding Walk
 *
 * Madras2013, The Self-Avoiding Walk, p. 308 ff (doi 10.1007/978-1-4614-6025-1_9)
 *
 * \param N number of steps
 * \return a list of random numbers from which the random lattice walk can be constructed
 */
std::list<double> SelfAvoidingWalker::dim(int N)
{
    int threshold = 10;
    if(N <= threshold)
    {
        std::list<double> start;

        // generate naively
        do
        {
            start.clear();
            double rn = rng();
            start.push_back(rn);
            Step<int> lastStep(d, rn);

            for(int i=1; i<N;)
            {
                double rn2 = rng();
                Step<int> s(d, rn2);
                if(s != -lastStep)  // do not allow immediate reversals
                {
                    ++i;
                    start.push_back(rn2);
                    lastStep = std::move(s);
                }
            }
        } while(!checkOverlapFree(start));
        return start;
    }
    else
    {
        std::list<double> part1;
        std::list<double> part2;

        do
        {
            part1 = dim(N/2);
            part2 = dim(N - N/2);

            // concat the lists
            part1.splice(part1.end(), part2);
        } while(!checkOverlapFree(part1));

        return part1;
    }
}
