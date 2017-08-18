#include "ScentWalker.hpp"

ScentWalker::ScentWalker(int d, int numSteps, int numWalker_in, int sideLength_in, int Tas_in, UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<int>(d, numSteps, rng, hull_algo, amnesia),
      numWalker(numWalker_in),
      sideLength(sideLength_in),
      Tas(Tas_in)
{
    random_numbers = rng.vector(numSteps*numWalker);
    histograms = std::vector<HistogramND>(numWalker, HistogramND(sideLength, d, 0, sideLength));
    newStep = Step<int>(d);
    undoStep = Step<int>(d);

    pos.resize(numWalker);
    for(auto &k : pos)
        k.resize(numSteps, Step<int>(d));
    for(auto &h : histograms)
        h.reset();

    for(int j=0; j<numWalker; ++j)
    {
        std::vector<int> init;
        for(int k=0; k<d; ++k)
            init.push_back(rng() * sideLength);

        starts.emplace_back(init);
    }

    init();
}

void ScentWalker::updateSteps()
{
    // use numWalker vectors for the steps (scent traces will be steps[now-Tas:now])
    // every walker has its own history

    // data structures: hashmap: site -> (map: who -> when)
    std::unordered_map<Step<int>, std::map<int, int>> trail(sideLength*sideLength);

    std::vector<int> to_remove;

    // start the agent simulation
    // init with random positions
    for(int j=0; j<numWalker; ++j)
        pos[j][0] = starts[j];

    // iterate the time, every agent does one move each timestep
    for(int i=0; i<numSteps-1; ++i)
        for(int j=0; j<numWalker; ++j)
        {
            auto &current = trail[pos[j][i]];
            to_remove.clear();
            //  at every visit remove expired entries from the back of the deque
            //  and entries of oneself (because oneself left a new scent in that moment)
            if(current.count(j))
                current[j] = i; // update last visited
            else
                current.emplace(j, i);

            for(auto &k : current)
                if(k.second < i-Tas)
                    to_remove.push_back(k.first);

            for(auto &k : to_remove)
                current.erase(k);

            // if we are on a foreign scent: retreat
            // if there is more than one marker (one marker is from us)
            if(current.size() > 1 && i > 0)
            {
                pos[j][i+1] = pos[j][i-1];
            }
            else
            {
                // else do a random step
                pos[j][i+1].fillFromRN(random_numbers[i*numWalker + j]);
                pos[j][i+1] += pos[j][i];
                pos[j][i+1].periodic(sideLength);
            }
            histograms[j].add(pos[j][i+1]);
        }

    // also, we need periodic boundaries and a fixed size on this one

    // populate the histogram (for a figure as in the artikel)

    // maybe update the points inside this function
    // if they are always up to date, `updatePoints` can be a noop

    // copy steps and points of walker 0 to m_steps and m_points
    // and everything will work -- though with a bit of overhead
    m_points = pos[0];
}

void ScentWalker::updatePoints(const int /*start*/)
{
    // points are always up to date, see `ScentWalker::updateSteps()`
}

void ScentWalker::change(UniformRNG &rng, bool update)
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
        // FIXME: problems with not-full dimensional walks
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

void ScentWalker::undoChange()
{
    random_numbers[undo_index] = undo_value;
    // test if something changed
    if(newStep == undoStep)
        return;

    updateSteps();
    updatePoints();
    m_convex_hull = m_old_convex_hull;
}

void ScentWalker::svg(const std::string filename, const bool with_hull) const
{
    // TODO
    LOG(LOG_WARNING) << "not yet implemented";

    SVG pic(filename);
    int idx = 0;
    for(const auto &p : pos)
    {
        std::vector<std::vector<double>> points;

        for(auto i : p)
        {
            auto x1 = i[0], y1 = i[1];
            std::vector<double> point {(double) x1, (double) y1};

            pic.circle(x1, y1, true, COLOR[idx%COLOR.size()]);

            points.push_back(point);
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

        ++idx;
    }

    if(d > 2)
        pic.text(0, sideLength-20, "projected from d=" + std::to_string(d), "red");

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
    pic.setGeometry(-1, -1, sideLength + 1, sideLength + 1);
    pic.save();

    svg_histogram("histo_" + filename);
}

void ScentWalker::svg_histogram(const std::string filename) const
{
    SVG pic(filename);

    // find the global maximum
    int maximum = 0;
    for(int i=0; i<numWalker; ++i)
    {
        int m = histograms[i].max();
        if(m > maximum)
            maximum = m;
    }

    // place rectangles for every bin
    // color should scale with number of entries
    // histograms of different walkers need to be merged
    for(int i=0; i<numWalker; ++i)
        for(int x=0; x<sideLength; ++x)
            for(int y=0; y<sideLength; ++y)
            {
                auto &data = histograms[i].get_data();
                // ignore not-visited fields
                if(!data[x*sideLength + y])
                    continue;

                // color should scale with number of entries
                std::string color = COLOR[i%COLOR.size()];
                double opacity = std::log(data[x*sideLength + y]) / std::log(maximum);
                pic.square(x, y, 1., color, opacity);
            }

    if(d > 2)
        pic.text(0, sideLength-20, "projected from d=" + std::to_string(d), "red");

    pic.setGeometry(0, 0, sideLength+1, sideLength+1);
    pic.save();
}
