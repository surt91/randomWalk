#include "SelfAvoidingWalker.hpp"

const std::vector<Step> SelfAvoidingWalker::steps() const
{
    if(!stepsDirty)
        return m_steps;

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

    return m_steps;
}

double SelfAvoidingWalker::rnChange(const int idx, const double other)
{
    // use this for a pivoting change?
    // or for some local change, or slithering snake?
    double oldRN = 0.0;
    return oldRN;
}

bool SelfAvoidingWalker::checkOverlapFree(std::list<Step> &l) const
{
    std::unordered_set<Step> steps;
    steps.reserve(l.size());

    auto it(l.begin());
    Step p(std::vector<int>(d, 0));
    while(it != l.end())
    {
        p += *it;
        if(steps.count(p))
            return false;
        steps.insert(p);
        ++it;
    }
    return true;
}

// Madras2013, The Self-Avoiding Walk, p. 308 ff
std::list<Step> SelfAvoidingWalker::dim(int N)
{
    // FIXME: do not save the steps, but only the random numbers
    //        and init from a sequence of random numbers like the others
    //        that way, the degeneration and serialization will work
    int threshold = 10;
    if(N <= threshold)
    {
        std::list<Step> start;
        // generate naively
        do
        {
            start.clear();
            double rn = rng();
            start.push_back(Step(d, rn));

            for(int i=1; i<N;)
            {
                Step s(d, rng());
                if(s != -start.back())  // do not allow immediate reversals
                {
                    ++i;
                    start.push_back(std::move(s));
                }
            }
        } while(!checkOverlapFree(start));
        //~ std::cout << start << std::endl;
        return start;
    }
    else
    {
        std::list<Step> part1;
        std::list<Step> part2;

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
