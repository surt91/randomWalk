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

SelfAvoidingWalker::SelfAvoidingWalker(int d, int numSteps, const UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<int>(d, numSteps, rng_in, hull_algo, amnesia)
{
    overlap_test.reserve(numSteps);

    generate_from_MCMC();
}

/// Get new random numbers and reconstruct the walk
void SelfAvoidingWalker::reconstruct()
{
    // start with a straight line
    generate_from_MCMC();
}

/// special initialization for SAW: use dimerization (with exponential runtime)
void SelfAvoidingWalker::generate_from_dimerization()
{
    auto l(dim(numSteps));
    random_numbers = std::vector<double>(l.begin(), l.end());
    init();
}

/// special initialization for SAW: use MCMC and pivot
void SelfAvoidingWalker::generate_from_MCMC()
{
    // start with a straight line
    random_numbers = std::vector<double>(numSteps, 0.);
    init();
    // TODO: MCMC to generate an equilibrated configuration
    // problem: How to detect, when I am equilibrated?
    // just do N pivots and hope for the best?
    // generally this should be ok, since it is just a starting condition
    // for the real MCMC
    // TODO: compare simple sampling results
    // we do 2*numSteps, since every second change is a naive one
    for(int i=0; i<2*numSteps; ++i)
        change(rng, true);
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
    // 50% pivot chance, 50% naive change
    // pivot not implemented for d >= 4
    double decision = rng();
    if(decision > 0.5) // 50%
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
                LOG(LOG_WARNING) << "Pivot algorithm only implemented for d<=3, "
                                    "will only use naive changes";
        }
        pivot(idx, symmetry, update);
    }
    else // 50%
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
 *
 * \image html pivot.svg "example of a pivot move"
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
            LOG(LOG_ERROR) << "Pivot algorithm only implemented for d<=4";
            throw std::invalid_argument("Pivot algorithm only implemented for d<=4");
    }

    // FIXME: pivot the shorter end
    // first just a dry test -- most of the time it will fail and it is
    // probably cheaper to do it twice in case of success instead of
    // undo much work everytime
    overlap_test.clear();
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

/** Set the random numbers such that we fill a hypercube.
 *
 * This will only be exact if the number of steps is a power of the
 * dimension, but should be good enough for all applications in this
 * context, which is mainly, estimating limits and equilibration.
 */
void SelfAvoidingWalker::degenerateMinSurface()
{
    int len = pow(numSteps, 1.0/d); // length of one side, rounded down
    int k = 0;
    std::vector<int> counter(d+1, 1);
    while(k < numSteps)
    {
        if(counter[1]%2)
            random_numbers[k] = 0.5/d;
        else
            random_numbers[k] = 0.0;
        ++k;

        for(int i=1; i<d; ++i)
            if(k > std::pow((double)len, (double)i) * counter[i])
            {
                if(k>=numSteps)
                    break;

                counter[i] += 1;
                if(counter[i+1]%2)
                    random_numbers[k] = (1.0/d) * i;
                else
                    random_numbers[k] = (1.0/d) * i + 0.5/d;
                ++k;
            }
    }

    updateSteps();
    updatePoints();
    updateHull();
}

void SelfAvoidingWalker::svgOfPivot(std::string filename)
{
    if(d != 2)
    {
        LOG(LOG_ERROR) << "Pivot algorithm visualization only implemented for d=2";
        throw std::invalid_argument("Pivot algorithm visualization only implemented for d=2");
    }


    SVG pic(filename);
    std::vector<std::vector<double>> ps;
    int min_x=0, max_x=0, min_y=0, max_y=0;
    for(auto i : points())
    {
        int x1 = i[0], y1 = i[1];
        std::vector<double> point {(double) x1, (double) y1};

        pic.circle(x1, y1, true, "gray");

        ps.push_back(point);

        if(x1 < min_x)
            min_x = x1;
        if(x1 > max_x)
            max_x = x1;
        if(y1 < min_y)
            min_y = y1;
        if(y1 > max_y)
            max_y = y1;
    }
    pic.polyline(ps, false, "grey");
    ps.clear();

    LOG(LOG_INFO) << points();

    int idx;
    int symmetry;
    do
    {
        idx = rng() * nRN();
        symmetry = rng() * 4; // integer between 0 and 3
    }
    while(!pivot(idx, symmetry, true));

    LOG(LOG_INFO) << idx << " " << symmetry;
    LOG(LOG_INFO) << points();

    for(auto i : points())
    {
        int x1 = i[0], y1 = i[1];
        std::vector<double> point {(double) x1, (double) y1};

        pic.circle(x1, y1, true);

        ps.push_back(point);

        if(x1 < min_x)
            min_x = x1;
        if(x1 > max_x)
            max_x = x1;
        if(y1 < min_y)
            min_y = y1;
        if(y1 > max_y)
            max_y = y1;
    }
    pic.polyline(ps);

    if(d > 2)
        pic.text(min_x, max_y-20, "projected from d=" + std::to_string(d), "red");

    pic.setGeometry(min_x -1, min_y - 1, max_x + 1, max_y + 1);
    pic.save();
}
