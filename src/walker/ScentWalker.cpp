#include "ScentWalker.hpp"

ScentWalker::ScentWalker(int d, int numSteps, int numWalker_in, int sideLength_in, int Tas_in, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<int>(d, numSteps, rng, hull_algo, amnesia),
      numWalker(numWalker_in),
      sideLength(sideLength_in),
      Tas(Tas_in),
      // relax(2*Tas),
      relax(0),
      periodic(false),
      circleStart(true)
{
    // TODO: pass relax as parameter
    LOG(LOG_INFO) << "This type needs to relax first, " << relax
                  << " additional steps will be simulated.";

    m_steps.resize(numSteps);

    histograms = std::vector<HistogramND>(numWalker,
                                HistogramND(sideLength, d, 0, sideLength));

    pos = std::vector<Step<int>>(numWalker, Step<int>(d));
    step = std::vector<Step<int>>(numWalker, Step<int>(d));

    reconstruct();
}

void ScentWalker::reconstruct()
{
    starts.clear();
    pos.clear();
    step.clear();
    for(auto &h : histograms)
        h.reset();

    if(circleStart)
    {
        int radius = std::min(sideLength/3, 2*numWalker);
        double phi_incr = 2.*M_PI/numWalker;
        LOG(LOG_INFO) << "starts on a circle with radius " << radius << " and angle increment " << phi_incr;
        for(int j=0; j<numWalker; ++j)
        {
            std::vector<int> tmp_start(d);

            int mid = sideLength/2;
            double phi = j*phi_incr;
            int x = radius * std::cos(phi) + mid;
            int y = radius * std::sin(phi) + mid;
            tmp_start[0] = x;
            tmp_start[1] = y;

            starts.emplace_back(std::move(tmp_start));
        }
    }
    else
    {
        std::set<Step<int>> occupied_starts;
        std::vector<int> tmp_start(d);
        for(int j=0; j<numWalker; ++j)
        {
            for(int k=0; k<d; ++k)
                tmp_start[k] = rng() * sideLength;

            // test if this place is alreay occupied (low probability, but it happens)
            if(occupied_starts.find(Step<int>(tmp_start)) != occupied_starts.end())
            {
                --j;
                continue;
            }
            occupied_starts.emplace(tmp_start);
            starts.emplace_back(tmp_start);
        }
    }

    if(!amnesia)
        random_numbers = rng.vector((numSteps+relax)*numWalker);

    init();
}

void ScentWalker::updateField(Site &site, int time)
{
    // static -> vector will be allocated only once
    static std::vector<int> to_remove;
    to_remove.clear();

    for(const auto &k : site)
        if(k.second < time-Tas)
            to_remove.push_back(k.first);

    for(auto &k : to_remove)
        site.erase(k);
}

void ScentWalker::updateSteps()
{
    // use numWalker vectors for the steps (scent traces will be steps[now-Tas:now])
    // every walker has its own history
    // static -> will be allocated only once
    static Field trail(sideLength*sideLength);
    trail.clear();

    // start the agent simulation
    for(int j=0; j<numWalker; ++j)
    {
        // init with random positions
        pos[j] = starts[j];
        // reset the histograms
        histograms[j].reset();
    }

    // iterate the time, every agent does one move each timestep
    for(int i=0; i<numSteps+relax; ++i)
    {
        for(int j=0; j<numWalker; ++j)
        {
            auto &current = trail[pos[j]];

            //  at every visit remove expired entries from the back of the deque
            //  and entries of oneself (because oneself left a new scent in that moment)
            current[j] = i; // update last visited

            updateField(current, i);

            // if we are on a foreign scent: retreat
            // if there is more than one marker (one marker is from us)
            if(current.size() > 1 && i > 0)
            {
                std::vector<Step<int>> candidates;
                for(const auto &k : pos[j].neighbors())
                {
                    // retreat only on own scent to not trap yourself behind a bridge
                    if(trail[k].size() == 1 && trail[k].find(j) != trail[k].end())
                        candidates.push_back(k);
                }

                if(candidates.size() == 0)
                {
                    // we are stuck, so just intrude into the other territory, I guess
                    if(!amnesia)
                        step[j].fillFromRN(random_numbers[i*numWalker + j]);
                    else
                        step[j].fillFromRN(rng());
                }
                else
                {
                    int idx;
                    if(!amnesia)
                        idx = random_numbers[i*numWalker + j] * candidates.size();
                    else
                        idx = rng() * candidates.size();
                    step[j] = candidates[idx] - pos[j];
                }

                pos[j] += step[j];
            }
            else
            {
                // else do a random step
                if(!amnesia)
                    step[j].fillFromRN(random_numbers[i*numWalker + j]);
                else
                    step[j].fillFromRN(rng());
                pos[j] += step[j];
            }

            // FIXME: this should be hidden
            for(int k=0; k<pos[j].d(); ++k)
            {
                if(pos[j][k] >= sideLength)
                {
                    pos[j][k] = sideLength - 2;
                    step[j].invert();
                }
                else if(pos[j][k] < 0)
                {
                    pos[j][k] = 1;
                    step[j].invert();
                }
            }

            // populate the histogram (for a figure as in the article)
            // if(i >= numSteps - relax)
            // {
                // TODO: maybe just the last relax many steps?
                histograms[j].add(pos[j]);
            // }
            if(i >= relax)
            {
                if(j == 0)
                    m_steps[i-relax] = step[j];
            }
        }
    }
}

void ScentWalker::change(UniformRNG &rng, bool update)
{
    // I should do this in a far more clever way

    // variable starts:
    // int idx = rng() * (nRN()+1) - numWalker;
    // fixed starts:
    int idx = rng() * (nRN()+1);

    // in the case of idx < 0, change the starting position of the (|idx|-1)-th walk
    undo_index = idx;
    if(idx < 0)
    {
        undo_start = starts[abs(idx)-1];

        std::vector<int> tmp_start(d);
        for(int k=0; k<d; ++k)
            tmp_start[k] = rng() * sideLength;

        starts[abs(idx)-1] = Step<int>(tmp_start);
    }
    else
    {
        undo_value = random_numbers[idx];
        random_numbers[idx] = rng();
    }

    updateSteps();
    updatePoints();

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

void ScentWalker::undoChange()
{
    if(undo_index < 0)
    {
        starts[abs(undo_index)-1] = undo_start;
    }
    else
    {
        random_numbers[undo_index] = undo_value;
    }

    updateSteps();
    updatePoints();
    m_convex_hull = m_old_convex_hull;
}

void ScentWalker::svg(const std::string filename, const bool with_hull) const
{
    SpecWalker::svg(filename, with_hull);

    histograms[0].svg("histo0_" + filename);

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
                opacity = std::max(0.1, opacity);
                pic.square(x, y, 1., color, opacity);
            }

    if(d > 2)
        pic.text(0, sideLength-20, "projected from d=" + std::to_string(d), "red");

    pic.setGeometry(0, 0, sideLength+1, sideLength+1);
    pic.save();
}

void ScentWalker::gp(const std::string filename, const bool /*with_hull*/) const
{
    GnuplotContour pic(filename);

    pic.data(histograms, starts);

    pic.save();
}
