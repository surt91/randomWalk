#include "LoopErasedWalker.hpp"

const std::vector<Step> LoopErasedWalker::steps() const
{
    if(!stepsDirty)
        return m_steps;

    // add steps
    // save their coordinates in a hashmap with their index
    // if a new coordinate is already in the hashmap, rease the loop
    // by setting the current position back to the index obtained from the
    // hashmap. Also clear the hashmap from all entries with a greater index
    // by iterating over the vector of steps taken, starting at index+1
    // then continue

    int N = random_numbers.size();
    std::unordered_map<Step, int> occupied_tiles;
    // at most as many steps as random numbers, i.e. no loop erasure
    std::vector<Step> ret(numSteps+1);

    // p will keep track where the head is
    Step p(std::vector<int>(d, 0));
    ret[0] = p; // start at origin
    occupied_tiles.insert({p, 0});

    int i=1;
    int index=1;
    while(index <= numSteps)
    {
        // generate more random numbers if necessary
        if(i >= N)
        {
            N *= 2;
            random_numbers.resize(N);

            std::generate(random_numbers.begin() + i, random_numbers.end(), rng);
        }
        Step s(d, random_numbers[i]);
        p += s;

        // if already occupied, erase loop
        auto it = occupied_tiles.find(p);
        if(it != occupied_tiles.end())
        {
            int tmp = it->second;
            for(int j=tmp+1; j<index; ++j)
            {
                // loop -> p is the same
                p += ret[j];
                occupied_tiles.erase(p);
            }
            p += s;
            index = tmp;
        }
        else
        {
            ret[index] = s;
            occupied_tiles.insert({p, index});
        }

        ++i;
        ++index;
    }
    log<LOG_INFO>("Random numbers used:") << i;

    m_steps = ret;
    stepsDirty = false;
    return m_steps;
}

double LoopErasedWalker::rnChange(const int idx, const double other)
{
    // I should do this in a far more clever way
    double tmp = random_numbers[idx];
    random_numbers[idx] = other;

    Step newStep(d, other);
    // test if something changes
    if(newStep == m_steps[idx])
        return tmp;

    stepsDirty = true;
    pointsDirty = true;
    hullDirty = true;

    return tmp;
}
