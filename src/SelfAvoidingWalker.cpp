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
//  pivot algorithm
//      generate somehow one SAW (dimerization), choose a pivot and
//      turn everything behind it clock-, or counterclockwise
//      + generates many SAW instances
//      + could be used as the "change" step of the large deviation scheme
//      - could introduce too much change
//      - how do I combine large deviation with a sampling technique?
//
//  pruned enriched Rosenbluth Rosenbluth (PERM)
//      grow SAWs with weights, create a population of SAWs
//      by duplicating or killing them based on their weight
//      if a walk reaches the length of N, take it as a sample
//      and continue with the next in the population
//      + generates many SAW instances, quite efficiently
//      - where can I plug in the large deviation "bias"? In the weights? How?

void SelfAvoidingWalker::change(UniformRNG &rng)
{
    // do a pivot change
    // choose the pivot
    int idx = rng() * nRN();
    undo_index = idx;
    int symmetry = rng() * 4; // integer between 0 and 3
    undo_value = symmetry;
    if(undo_value == 2)
        undo_value = 3;
    if(undo_value == 3)
        undo_value = 2;

    pivot(idx, symmetry);

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

void SelfAvoidingWalker::pivot(const int index, const int op)
{
    std::vector<int> matrix(d*d, 0);
    // FIXME: implement for d > 2
    if(d != 2)
        throw std::invalid_argument("Pivot algorithm only implemented for d=2");
    // choose the symmetry operation
    switch(op)
    {
        // reflection on x-axis
        case 0:
            matrix[0] = 1;
            matrix[3] = -1;
            break;
        // reflection on y-axis
        case 1:
            matrix[0] = -1;
            matrix[3] = 1;
            break;
        // rotate by pi/2
        case 2:
            matrix[1] = 1;
            matrix[2] = -1;
            break;
        // rotate by -pi/2
        case 3:
            matrix[1] = -1;
            matrix[2] = 1;
            break;
    }

    steps(); // make sure steps is up to date
    // FIXME, pivot the shorter end
    // first just a dry test -- most of the time it will fail and it is
    // probably cheaper to do it twice in case of success instead of
    // undo much work everytime
    std::unordered_set<Step> overlap_test;
    overlap_test.reserve(numSteps);
    Step position(std::vector<int>(d, 0));
    overlap_test.insert(position);
    bool failed = false;
    for(int i=0; i<index; ++i)
    {
        position += m_steps[i];
        overlap_test.insert(position);
    }

    for(int i=index; i<numSteps; ++i)
    {
        position += transform(m_steps[i], matrix);

        if(overlap_test.count(position))
        {
            failed = true;
            break;
        }
        overlap_test.insert(position);
    }
    if(!failed)
        for(int i=index; i<numSteps; ++i)
            m_steps[i] = transform(m_steps[i], matrix);

    pointsDirty = true;
    hullDirty = true;
}

void SelfAvoidingWalker::undoChange()
{
    pivot(undo_index, undo_value);

    pointsDirty = true;
    hullDirty = true;
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
