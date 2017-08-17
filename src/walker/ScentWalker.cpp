#include "ScentWalker.hpp"

// TODO this does not seem very abstract -> rename to ScentWalker

ScentWalker::ScentWalker(int d, int numSteps, int numWalker_in, int sideLength_in, int Tas_in, UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<int>(d, numSteps, rng, hull_algo, amnesia),
      numWalker(numWalker_in),
      sideLength(sideLength_in),
      Tas(Tas_in)
{
    random_numbers = rng.vector(numSteps*numWalker);
    init();
}

void ScentWalker::updateSteps()
{
    // use numWalker vectors for the steps (scent traces will be steps[now-Tas:now])
    // every walker has its own history
    pos.clear();
    pos.resize(numWalker);
    for(auto &k : pos)
        k.resize(numSteps, Step<int>(d));

    // data structures: hashmap/bitmap: site -> deque[(time of last visit, who visited)]
    // data structures: hashmap: site -> (map: who -> when)
    std::unordered_map<Step<int>, std::map<int, int>> trail(sideLength*sideLength);

    std::vector<int> to_remove;

    // TODO
    // start the agent simulation
    // init with random positions
    for(int j=0; j<numWalker; ++j)
    {
        std::vector<int> init;
        init.push_back(rng() * sideLength);
        init.push_back(rng() * sideLength);
        pos[j][0] = Step<int>(init);
    }
    // iterate the time, every agent does one move each timestep
    for(int i=0; i<numSteps-1; ++i)
    {
        for(int j=0; j<numWalker; ++j)
        {
            auto &current = trail[pos[j][i]];
            to_remove.clear();
            //  at every visit remove expired entries from the back of the deque
            //  and entries of oneself (because oneself left a new scent in that moment)
            if(current.count(j))
            {
                // update last visited
                current[j] = i;
            }
            else
            {
                current.emplace(j, i);
            }

            for(auto &k : current)
            {
                if(k.second < i-Tas)
                {
                    to_remove.push_back(k.first);
                }
            }

            for(auto &k : to_remove)
            {
                current.erase(k);
            }

            // if we are on a foreign scent: retreat
            // if there is more than one marker (one marker is from us)
            if(current.size() > 1)
            {
                pos[j][i+1] = pos[j][i-1];
            }
            else
            {
                // else do a random step
                pos[j][i+1].fillFromRN(rng());
                pos[j][i+1] += pos[j][i];
            }
        }
    }

    // also, we need periodic boundaries and a fixed size on this one

    // populate the histogram (for a figure as in the artikel)

    // maybe update the points inside this function
    // if they are always up to date, `updatePoints` can be a noop

    // copy steps and points of walker 0 to m_steps and m_points
    // and everything will work -- though with a bit of overhead
}

void ScentWalker::updatePoints(const int /*start*/)
{
    // points are always up to date, see `ScentWalker::updateSteps()`
}

void ScentWalker::change(UniformRNG &rng, bool update)
{
    LOG(LOG_WARNING) << "not yet implemented";
}

void ScentWalker::undoChange()
{
    LOG(LOG_WARNING) << "not yet implemented";
}

void ScentWalker::svg(const std::string filename, const bool with_hull) const
{
    SVG pic(filename);
    int min_x=0, max_x=0, min_y=0, max_y=0;
    int idx = 0;
    for(const auto &p : pos)
    {
        ++idx;
        std::vector<std::vector<double>> points;

        for(auto i : p)
        {
            auto x1 = i[0], y1 = i[1];
            std::vector<double> point {(double) x1, (double) y1};

            pic.circle(x1, y1, true, COLOR[idx%COLOR.size()]);

            points.push_back(point);

            if(x1 < min_x)
                min_x = x1;
            if(x1 > max_x)
                max_x = x1;
            if(y1 < min_y)
                min_y = y1;
            if(y1 > max_y)
                max_y = y1;
        }
        pic.polyline(points, false, COLOR[idx%COLOR.size()]);

        // points.clear();
        // if(with_hull)
        // {
        //     const auto h = w.hullPoints();
        //     for(auto &i : h)
        //     {
        //         std::vector<double> point {(double) i[0], (double) i[1]};
        //         points.push_back(point);
        //     }
        //     pic.polyline(points, true, COLOR[idx%COLOR.size()]);
        // }
    }

    if(d > 2)
        pic.text(min_x, max_y-20, "projected from d=" + std::to_string(d), "red");

    // if(with_hull)
    // {
    //     std::vector<std::vector<double>> points;
    //     const auto h = m_convex_hull.hullPoints();
    //     for(auto &i : h)
    //     {
    //         std::vector<double> point {(double) i[0], (double) i[1]};
    //         points.push_back(point);
    //     }
    //     pic.polyline(points, true, std::string("red"));
    // }
    pic.setGeometry(min_x -1, min_y - 1, max_x + 1, max_y + 1);
    pic.save();
}
