#include "LoopErasedWalker.hpp"

void LoopErasedWalker::updateSteps()
{
    // add steps
    // save their coordinates in a hashmap with their index
    // if a new coordinate is already in the hashmap, rease the loop
    // by setting the current position back to the index obtained from the
    // hashmap. Also clear the hashmap from all entries with a greater index
    // by iterating over the vector of steps taken, starting at index+1
    // then continue

    int N = random_numbers.size();
    std::unordered_map<Step<int>, int> occupied_tiles;
    // at most as many steps as random numbers, i.e. no loop erasure
    std::vector<Step<int>> ret(numSteps);

    // p will keep track where the head is
    Step<int> p(d);
    occupied_tiles.emplace(p, 0);

    int i=0;
    int index=0;
    while(index < numSteps)
    {
        // generate more random numbers if necessary
        if(i >= N)
        {
            N *= 2;
            random_numbers.resize(N);

            std::generate(random_numbers.begin() + i, random_numbers.end(), rng);
        }
        Step<int> s(d, random_numbers[i]);
        p += s;

        // if already occupied, erase loop
        auto it = occupied_tiles.find(p);
        if(it != occupied_tiles.end())
        {
            int tmp = it->second;
            // zero is a special case
            if(tmp == 0)
            {
                occupied_tiles.clear();
                p = Step<int>(d);
                occupied_tiles.emplace(p, 0);
                index = -1;
            }
            else
            {
                //~ std::cout << "tmp " << tmp << std::endl;
                for(int j=tmp+1; j<index; ++j)
                {
                    // loop -> p is the same
                    p += ret[j];
                    occupied_tiles.erase(p);
                }
                p += s;
                index = tmp;
            }
        }
        else
        {
            ret[index] = s;
            occupied_tiles.emplace(p, index);
        }

        ++index;
        ++i;
    }
    random_numbers_used = i;
    LOG(LOG_TOO_MUCH) << "Random numbers used: " << random_numbers_used;

    m_steps = ret;
}

int LoopErasedWalker::nRN() const
{
    return random_numbers_used;
}

void LoopErasedWalker::change(UniformRNG &rng)
{
    // I should do this in a far more clever way
    int idx = rng() * nRN();
    undo_index = idx;
    undo_value = random_numbers[idx];
    random_numbers[idx] = rng();

    Step<int> newStep(d, random_numbers[idx]);
    // test if something changes
    if(newStep == Step<int>(d, undo_value))
        return;

    updateSteps();
    updatePoints();
    updateHull();
}

void LoopErasedWalker::undoChange()
{
    double rejected = random_numbers[undo_index];
    random_numbers[undo_index] = undo_value;
    Step<int> newStep(d, undo_value);
    // test if something changes
    if(newStep == Step<int>(d, rejected))
        return;

    updateSteps();
    updatePoints();
    updateHull();
}