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

/// transformation matrices for pivoting d=4
static const int tMatrix4[][16] =
{
    // mirror at xyz-volume
        { 1,  0,  0,  0,
          0,  1,  0,  0,
          0,  0,  1,  0,
          0,  0,  0, -1},
    // mirror at xyw-volume
        { 1,  0,  0,  0,
          0,  1,  0,  0,
          0,  0, -1,  0,
          0,  0,  0,  1},
    // mirror at xzw-volume
        { 1,  0,  0,  0,
          0, -1,  0,  0,
          0,  0,  1,  0,
          0,  0,  0,  1},
    // mirror at yzw-volume
        {-1,  0,  0,  0,
          0,  1,  0,  0,
          0,  0,  1,  0,
          0,  0,  0,  1},
    // rotate by pi/2 around xy-plane
        { 1,  0,  0,  0,
          0,  1,  0,  0,
          0,  0,  0,  1,
          0,  0, -1,  0},
    // rotate by pi around xy-plane
        { 1,  0,  0,  0,
          0,  1,  0,  0,
          0,  0, -1,  0,
          0,  0,  0, -1},
    // rotate by -pi/2 around xy-plane
        { 1,  0,  0,  0,
          0,  1,  0,  0,
          0,  0,  0, -1,
          0,  0,  1,  0},
    // rotate by pi/2 around xz-plane
        { 1,  0,  0,  0,
          0,  0,  0,  1,
          0,  0,  1,  0,
          0, -1,  0,  0},
    // rotate by pi around xz-plane
        { 1,  0,  0,  0,
          0, -1,  0,  0,
          0,  0,  1,  0,
          0,  0,  0, -1},
    // rotate by -pi/2 around xz-plane
        { 1,  0,  0,  0,
          0,  0,  0, -1,
          0,  0,  1,  0,
          0,  1,  0,  0},
    // rotate by pi/2 around xw-plane
        { 1,  0,  0,  0,
          0,  0,  1,  0,
          0, -1,  0,  0,
          0,  0,  0,  1},
    // rotate by pi around xw-plane
        { 1,  0,  0,  0,
          0, -1,  0,  0,
          0,  0, -1,  0,
          0,  0,  0,  1},
    // rotate by -pi/2 around xw-plane
        { 1,  0,  0,  0,
          0,  0, -1,  0,
          0,  1,  0,  0,
          0,  0,  0,  1},
    // rotate by pi/2 around yz-plane
        { 0,  0,  0,  1,
          0,  1,  0,  0,
          0,  0,  1,  0,
         -1,  0,  0,  0},
    // rotate by pi around yz-plane
        {-1,  0,  0,  0,
          0,  1,  0,  0,
          0,  0,  1,  0,
          0,  0,  0, -1},
    // rotate by -pi/2 around yz-plane
        { 0,  0,  0, -1,
          0,  1,  0,  0,
          0,  0,  1,  0,
          1,  0,  0,  0},
    // rotate by pi/2 around yw-plane
        { 0,  0,  1,  0,
          0,  1,  0,  0,
         -1,  0,  0,  0,
          0,  0,  0,  1},
    // rotate by pi around yw-plane
        {-1,  0,  0,  0,
          0,  1,  0,  0,
          0,  0, -1,  0,
          0,  0,  0,  1},
    // rotate by -pi/2 around yw-plane
        { 0,  0, -1,  0,
          0,  1,  0,  0,
          1,  0,  0,  0,
          0,  0,  0,  1},
    // rotate by pi/2 around zw-plane
        { 0,  1,  0,  0,
         -1,  0,  0,  0,
          0,  0,  1,  0,
          0,  0,  0,  1},
    // rotate by pi around zw-plane
        {-1,  0,  0,  0,
          0, -1,  0,  0,
          0,  0,  1,  0,
          0,  0,  0,  1},
    // rotate by -pi/2 around zw-plane
        { 0, -1,  0,  0,
          1,  0,  0,  0,
          0,  0,  1,  0,
          0,  0,  0,  1},
};
/// inverse transformations
static const int iMatrix4[] =
{
    // mirrors
    0,
    1,
    2,
    3,
    // rotate xy
    6,
    5,
    4,
    //rotate xz
    9,
    8,
    7,
    //rotate xw
    12,
    11,
    10,
    //rotate yz
    15,
    14,
    13,
    //rotate yw
    18,
    17,
    16,
    //rotate zw
    21,
    20,
    19
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

/** Changes the walk, i.e., performs one trial move.
 *
 * One change can consist of a "naive change" (80%) or a "pivot change" (20%).
 * The "naive change" changes one step in the walk, such that all
 * points after that are moved.
 * The "pivot change" performs a symmetry operation at a random site in
 * the walk, i.e., a rotation or mirroring. This leads to faster decorrelation
 * but is seldom accepted.
 *
 * \param rng Random number generator to draw the needed randomness from
 * \param update Should the hull be updated after the change?
 */
void SelfAvoidingWalker::change(UniformRNG &rng, bool update)
{
    // do a pivot change
    // choose the pivot
    int idx = rng() * nRN();
    undo_index = idx;

    // choose the change algorithm randomly
    // 20% pivot chance, 80% naive change
    // pivot not implemented for d >= 4
    if(rng() > 0.8)
    {
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
            case 4:
                symmetry = rng() * 22; // integer between 0 and 21
                undo_symmetry = iMatrix4[symmetry];
                break;
            default:
                symmetry = -1;
                LOG(LOG_WARNING) << "Pivot algorithm only implemented for d<=3, will only use naive changes";
        }
        pivot(idx, symmetry, update);
    }
    else
    {
        undo_index = -1;
        int idx = rng() * nRN();
        auto val = rng();
        naiveChange(idx, val, update);
    }
}

/// Undoes the last change.
void SelfAvoidingWalker::undoChange()
{
    // which change was done
    if(undo_index == -1)
        naiveChangeUndo();
    else
        pivot(undo_index, undo_symmetry);
}

/**
 * Applies the transformation matrix m to the point p.
 *
 * \param p Point to be transformed.
 * \param m Transformation matrix to use for the tansform.
 * \return the transformed point
 */
Step<int> SelfAvoidingWalker::transform(const Step<int> &p, const int *m) const
{
    Step<int> out(d);
    for(int i=0; i<d; ++i)
        for(int j=0; j<d; ++j)
            out[i] += p[j] * m[i*d + j];

    return out;
}

/** Pivot Algorithm
 *
 * Madras2013, The Self-Avoiding Walk, p. 322 ff (doi 10.1007/978-1-4614-6025-1_9)
 *
 *  \param index The pivot site around which the symmetry operation is done.
 *  \param op The symmetry operation to be done (index of the transformation matrix)
 *  \param update Should the hull be updated after the transformation.
 *  \return Was the pivoting successful, or did it cross itself?
 */
bool SelfAvoidingWalker::pivot(const int index, const int op, bool update)
{
    const int* matrix;
    // FIXME: implement for d > 4

    // choose the symmetry operation
    switch(d)
    {
        case 2:
            matrix = tMatrix2[op];
            break;
        case 3:
            matrix = tMatrix3[op];
            break;
        case 4:
            matrix = tMatrix4[op];
            break;
        default:
            throw std::invalid_argument("Pivot algorithm only implemented for d<=4");
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
        if(update)
        {
            updateHull();
        }
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
    m_convex_hull = m_old_convex_hull;
}

bool SelfAvoidingWalker::naiveChange(const int idx, const double rn, bool update)
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

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }

    return true;
}

/** Silthering Snake
 *
 * Slithering Snake appends one step at the one side and removes the last
 * step on the other side. Note that this is not ergodic, since the snake
 * can be trapped. This, however, is mitigated by also using pivoting,
 * which is ergodic.
 *
 * Madras2013, The Self-Avoiding Walk, p. 320 ff (doi 10.1007/978-1-4614-6025-1_9)
 *
 *  \param front does sthe snake slither forwards or backwards
 *  \param rn random number to determine the direction of the slithering
 *  \return was it successful, or did the walk cross itself?
 */
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

//~ /** set the random numbers such that we get an one dimensional line
 //~ */
//~ template <>
//~ inline void SelfAvoidingWalker::degenerateMinVolume()
//~ {
    //~ for(int i=0; i<numSteps; ++i)
        //~ random_numbers[i] = .99;

    //~ updateSteps();
    //~ updatePoints();
    //~ updateHull();
//~ }

//~ /** set the random numbers such that we always step left, right, left, right
 //~ */
//~ template <>
//~ inline void SpecWalker<int>::degenerateMinSurface()
//~ {
    //~ for(size_t i=0; i<random_numbers.size(); ++i)
        //~ random_numbers[i] = i % 2 ? .99 : .99 - 1/d;

    //~ updateSteps();
    //~ updatePoints();
    //~ updateHull();
//~ }
