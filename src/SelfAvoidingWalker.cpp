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
            symmetry = rng() * tMatrix2.size(); // integer between 0 and 3
            undo_value = iMatrix2[symmetry];
            break;
        case 3:
            symmetry = rng() * tMatrix3.size(); // integer between 0 and 3
            undo_value = iMatrix3[symmetry];
            break;
        default:
            throw std::invalid_argument("Pivot algorithm only implemented for d=2 and d=3");
    }

    // do only recalculate, if pivot was successful
    if(pivot(idx, symmetry))
    {
        pointsDirty = true;
        hullDirty = true;
    }
    else
        undo_index = -1; // flag, that undo is not possible/needed
}

void SelfAvoidingWalker::undoChange()
{
    // no change was made (because the pivot failed)
    if(undo_index == -1)
        return;

    pivot(undo_index, undo_value);

    pointsDirty = true;
    hullDirty = true;
}

Step SelfAvoidingWalker::transform(Step &p, const std::vector<int> &m) const
{
    Step out(std::vector<int>(d, 0));
    for(int i=0; i<d; ++i)
        for(int j=0; j<d; ++j)
            out[i] += p[j] * m[i*d + j];

    return out;
}

bool SelfAvoidingWalker::pivot(const int index, const int op)
{
    std::vector<int> matrix;
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
    }

    steps(); // make sure steps is up to date
    // FIXME: pivot the shorter end
    // first just a dry test -- most of the time it will fail and it is
    // probably cheaper to do it twice in case of success instead of
    // undo much work everytime
    std::unordered_set<Step> overlap_test;
    overlap_test.reserve(numSteps);
    Step positioni(std::vector<int>(d, 0));
    Step positionj(std::vector<int>(d, 0));
    overlap_test.insert(positioni);
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

        pointsDirty = true;
        hullDirty = true;
    }

    return !failed;
}

bool SelfAvoidingWalker::checkOverlapFree(std::list<double> &l) const
{
    std::unordered_set<Step> steps;
    steps.reserve(l.size());

    auto it(l.begin());
    Step p(std::vector<int>(d, 0));
    while(it != l.end())
    {
        p += Step(d, *it);
        if(steps.count(p))
            return false;
        steps.insert(p);
        ++it;
    }
    return true;
}

// Madras2013, The Self-Avoiding Walk, p. 308 ff (doi 10.1007/978-1-4614-6025-1_9)
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
            Step lastStep(d, rn);

            for(int i=1; i<N;)
            {
                double rn2 = rng();
                Step s(d, rn2);
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
