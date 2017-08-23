#include "ScentWalker.hpp"

ScentWalker::ScentWalker(int d, int numSteps, int numWalker_in, int sideLength_in, int Tas_in, UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<int>(d, numSteps, rng, hull_algo, amnesia),
      numWalker(numWalker_in),
      sideLength(sideLength_in),
      Tas(Tas_in),
      relax(2*Tas)
{
    // TODO: pass relax as parameter
    LOG(LOG_INFO) << "This type needs to relax first, " << relax << " additional steps will be simulated.";

    if(!amnesia)
        random_numbers = rng.vector((numSteps+relax)*numWalker);

    histograms = std::vector<HistogramND>(numWalker, HistogramND(sideLength+1, d, 0, sideLength));
    newStep = Step<int>(d);
    undoStep = Step<int>(d);

    m_steps = std::vector<Step<int>>(numSteps, Step<int>(d));

    pos = std::vector<Step<int>>(numWalker, Step<int>(d));
    step = std::vector<Step<int>>(numWalker, Step<int>(d));

    for(auto &h : histograms)
        h.reset();

    std::vector<int> tmp_start(d);
    for(int j=0; j<numWalker; ++j)
    {
        for(int k=0; k<d; ++k)
            tmp_start[k] = rng() * sideLength;

        starts.emplace_back(tmp_start);
    }

    init();
}

void ScentWalker::updateField(Site &site, int time)
{
    // static -> vector will be allocated only once
    static std::vector<int> to_remove;
    to_remove.clear();

    for(auto &k : site)
        if(k.second < time-Tas)
            to_remove.push_back(k.first);

    for(auto &k : to_remove)
        site.erase(k);
}

void ScentWalker::updateSteps()
{
    // use numWalker vectors for the steps (scent traces will be steps[now-Tas:now])
    // every walker has its own history

    Field trail(sideLength*sideLength);

    // start the agent simulation
    for(int j=0; j<numWalker; ++j)
    {
        // init with random positions
        pos[j] = starts[j];
        // reset the histograms
        histograms[j].reset();
    }

    // iterate the time, every agent does one move each timestep
    for(int i=0; i<numSteps+relax-1; ++i)
        for(int j=0; j<numWalker; ++j)
        {
            auto &current = trail[pos[j]];

            //  at every visit remove expired entries from the back of the deque
            //  and entries of oneself (because oneself left a new scent in that moment)
            current[j] = i; // update last visited

            updateField(current, i);

            // if we are on a foreign scent: retreat
            // if there is more than one marker (one marker is from us)
            // TODO: do not retreat directly but on a random neighboring field
            // withour the encountered scent
            if(current.size() > 1 && i > 0)
            {
                step[j].invert();
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
                pos[j].periodic(sideLength);
            }

            // populate the histogram (for a figure as in the articel)
            if(i > relax)
            {
                // TODO: maybe just the last relax many steps?
                histograms[j].add(pos[j]);
                m_steps.push_back(step[j]);
            }
        }
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
                double opacity = std::max(0.1, std::log(data[x*sideLength + y]) / std::log(maximum));
                pic.square(x, y, 1., color, opacity);
            }

    if(d > 2)
        pic.text(0, sideLength-20, "projected from d=" + std::to_string(d), "red");

    pic.setGeometry(0, 0, sideLength+1, sideLength+1);
    pic.save();
}

void ScentWalker::gp(const std::string filename, const bool with_hull) const
{
    GnuplotContour pic(filename);

    pic.data(histograms);

    pic.save();
}
