#include "LoopErasedWalker.hpp"

LoopErasedWalker::LoopErasedWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<int>(d, numSteps, rng, hull_algo, amnesia)
{
    newStep = Step<int>(d);
    undoStep = Step<int>(d);
    if(!amnesia)
        random_numbers = rng.vector(numSteps);
    init();
}

/// Get new random numbers and reconstruct the walk
void LoopErasedWalker::reconstruct()
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

void LoopErasedWalker::updateSteps()
{
    // add steps
    // save their coordinates in a hashmap with their index
    // if a new coordinate is already in the hashmap, erase the loop
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
    Step<int> s(d);
    occupied_tiles.emplace(p, 0);

    int i=0;
    int index=0;
    while(index < numSteps)
    {
        if(!amnesia)
        {
            // generate more random numbers if necessary
            if(i >= N)
            {
                N *= 2;
                random_numbers.resize(N);

                std::generate(random_numbers.begin() + i, random_numbers.end(), std::ref(rng));
            }
            s.fillFromRN(random_numbers[i]);
        }
        else
        {
            s.fillFromRN(rng());
        }
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
                p.setZero();
                occupied_tiles.emplace(p, 0);
                index = -1;
            }
            else
            {
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

void LoopErasedWalker::change(UniformRNG &rng, bool update)
{
    // I should do this in a far more clever way
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

void LoopErasedWalker::undoChange()
{
    random_numbers[undo_index] = undo_value;
    // test if something changed
    if(newStep == undoStep)
        return;

    updateSteps();
    updatePoints();
    m_convex_hull = m_old_convex_hull;
}

void LoopErasedWalker::svgOfErasedLoops(std::string filename)
{
    SVG pic(filename);
    const std::vector<Step<int>> p = points();
    std::vector<std::vector<double>> points;
    int min_x=0, max_x=0, min_y=0, max_y=0;
    Step<int> i(d);
    for(int j=0; j<nRN(); ++j)
    {
        i += Step<int>(d, random_numbers[j]);
        int x1 = i[0], y1 = i[1];
        std::vector<double> point {(double) x1, (double) y1};
        points.push_back(point);

        pic.circle(x1, y1, true, "grey");

        if(x1 < min_x)
            min_x = x1;
        if(x1 > max_x)
            max_x = x1;
        if(y1 < min_y)
            min_y = y1;
        if(y1 > max_y)
            max_y = y1;
    }
    pic.polyline(points, false, "grey");
    points.clear();
    for(auto i : p)
    {
        int x1 = i[0], y1 = i[1];
        std::vector<double> point {(double) x1, (double) y1};

        pic.circle(x1, y1, true);

        points.push_back(point);
    }
    pic.polyline(points);

    if(d > 2)
        pic.text(min_x, max_y-20, "projected from d=" + std::to_string(d), "red");

    pic.setGeometry(min_x -1, min_y - 1, max_x + 1, max_y + 1);
    pic.save();
}
