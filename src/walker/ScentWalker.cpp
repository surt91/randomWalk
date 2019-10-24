#include "ScentWalker.hpp"

ScentWalker::ScentWalker(int d, int numSteps, int numWalker_in, int sideLength_in, int Tas_in, agent_start_t start_configuration_in, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia, bool save_histograms_in)
    : SpecWalker<int>(d, numSteps, rng, hull_algo, amnesia),
      numWalker(numWalker_in),
      sideLength(sideLength_in),
      Tas(Tas_in),
      relax(start_configuration_in == AS_RELAXED ? Tas_in : 0),
      periodic(false),
      start_configuration(start_configuration_in),
      save_histograms(save_histograms_in)
{
    m_steps.resize(numSteps);

    updatedNumWalker();
    reconstruct();
}

void ScentWalker::updatedNumWalker()
{
    interaction_sets.resize(numWalker);

    if(save_histograms)
        histograms = std::vector<HistogramND>(
            numWalker,
            HistogramND(sideLength, d, 0, sideLength)
        );

    pos = std::vector<Step<int>>(numWalker, Step<int>(d));
    step = std::vector<Step<int>>(numWalker, Step<int>(d));
}

void ScentWalker::reconstruct()
{
    starts.clear();
    pos.clear();
    step.clear();

    if(save_histograms)
        for(auto &h : histograms)
            h.reset();

    if(start_configuration == AS_CIRCLE)
    {
        // double wanted_radius = pow(numSteps, 0.33)*4;
        // double wanted_radius = numSteps/10.;
        double wanted_radius = sqrt(numSteps);
        int radius = std::min(sideLength/2., wanted_radius);

        if(radius >= sideLength/2.)
        {
            LOG(LOG_WARNING) << "cutoff field size: should be "
                << wanted_radius << " but is " << radius;
        }

        double phi_incr = 2.*M_PI/(numWalker-1);
        LOG(LOG_DEBUG) << "starts on a circle with radius " << radius << " and angle increment " << phi_incr;
        int mid = sideLength/2;
        std::vector<int> middle(d, mid);
        starts.emplace_back(std::move(middle));
        for(int j=1; j<numWalker; ++j)
        {
            std::vector<int> tmp_start(d);

            double phi = j*phi_incr;
            int x = radius * std::cos(phi) + mid;
            int y = radius * std::sin(phi) + mid;
            tmp_start[0] = x;
            tmp_start[1] = y;

            starts.emplace_back(std::move(tmp_start));
        }
    }
    else if(start_configuration == AS_TRIANGULAR)
    {
        double lattice_constant = sqrt(numSteps)/2.;

        if(lattice_constant >= sideLength/2.)
        {
            LOG(LOG_WARNING) << "cutoff field size: should be less than "
                << (sideLength/2.) << " but is " << lattice_constant;
        }

        LOG(LOG_INFO) << "starts on a triangular lattice with lattice constant " << lattice_constant;
        int mid = sideLength/2;
        std::vector<int> middle(d, mid);
        starts.emplace_back(std::move(middle));

        /*
           we will place the other agents on a triangular lattice, with
           the walker of interest in the center, like this:

           o-o-o-o-
           \/\/\/\
           o-o-o-o
           \/\/\/\
           o-o-o-o

           We will raster all possible points in the square and perform a
           simple rejection method for points outside of the square.
           Therefore we use $e_1 = (1,0)$ and $e_2 = (-0.5,\sqrt(3)/2)$
           times the lattice constant as our coordinate vectors.
        */

        int max_e1 = sideLength / lattice_constant + 1;
        int min_e1 = -sideLength / lattice_constant - 1;
        numWalker = 1;
        for(double x1=min_e1; x1<max_e1; ++x1)
            for(double x2=min_e1; x2<max_e1; ++x2)
            {
                // the center is already our walk of interest
                if(x1 == 0 && x2 == 0)
                    continue;

                std::vector<int> tmp_start(d);
                // double x = x1 * 1.0 + x2 * -0.5;
                // double y = x1 * 0.0 + x2 * sqrt(3.)/2.;
                double x = (x1 + x2 * -0.5) * lattice_constant + mid;
                double y = (x2 * sqrt(3.)/2.) * lattice_constant + mid;
                if(x < 0 || x > sideLength || y < 0 || y > sideLength)
                    continue;

                ++numWalker;
                tmp_start[0] = std::round(x);
                tmp_start[1] = std::round(y);
                starts.emplace_back(std::move(tmp_start));
            }
        updatedNumWalker();
        LOG(LOG_INFO) << "number of other walkers: " << numWalker;
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
        if(start_configuration == AS_RELAXED)
        {
            // let the system run for T_as steps and save the
            // markers and positions as the frozen initial conditions
            // for the following simulation
            initial_trail.clear();
            for(int i=0; i<relax; ++i)
            {
                for(int j=0; j<numWalker; ++j)
                {
                    moveWalker(j, i, initial_trail, true);
                }
            }
        }
    }

    if(!amnesia)
        random_numbers = rng.vector(numSteps*numWalker);

    init();

    LOG(LOG_DEBUG) << "Interactions";
    for(int i=0; i<numWalker; ++i)
        LOG(LOG_DEBUG) << i << ": " << interactions(i);
}

int ScentWalker::interactions(int walkerId)
{
    return interaction_sets[walkerId].size();
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

void ScentWalker::moveWalker(int j, int i, Field &trail, bool forced_amnesia)
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
        // record with which adversary we interacted
        for(auto &field_entry : current)
        {
            int adversary = field_entry.first;
            if(adversary != j) // do not record yourself
                interaction_sets[j].insert(adversary);
        }

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
            if(!amnesia && !forced_amnesia)
                step[j].fillFromRN(random_numbers[i*numWalker + j]);
            else
                step[j].fillFromRN(rng());
        }
        else
        {
            int idx;
            if(!amnesia && !forced_amnesia)
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
        if(!amnesia && !forced_amnesia)
            step[j].fillFromRN(random_numbers[i*numWalker + j]);
        else
            step[j].fillFromRN(rng());
        pos[j] += step[j];
    }

    // FIXME: this should be hidden
    // boundary
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
}

void ScentWalker::updateSteps()
{
    // use numWalker vectors for the steps (scent traces will be steps[now-Tas:now])
    // every walker has its own history
    // static -> will be allocated only once
    static Field trail(std::min(sideLength*sideLength, numSteps*numWalker));
    // in the relaxed start configuration we have scent marks on the initial map
    if(start_configuration == AS_RELAXED)
    {
        trail = initial_trail;
    }
    else
    {
        trail.clear();
    }

    for(auto &i : interaction_sets)
        i.clear();

    // start the agent simulation
    for(int j=0; j<numWalker; ++j)
    {
        // init with starting positions
        pos[j] = starts[j];

        // reset the histograms
        if(save_histograms)
            histograms[j].reset();
    }

    // iterate the time, every agent does one move each timestep
    for(int i=relax; i<numSteps; ++i)
    {
        for(int j=0; j<numWalker; ++j)
        {
            moveWalker(j, i, trail);

            if(save_histograms)
                histograms[j].add(pos[j]);
            if(j == 0)
                m_steps[i-relax] = step[j];
        }
    }
}

void ScentWalker::change(UniformRNG &rng, bool update)
{
    // I should do this in a far more clever way

    int idx;
    // fixed starts:
    if(
        start_configuration == AS_CIRCLE ||
        start_configuration == AS_TRIANGULAR ||
        start_configuration == AS_RELAXED
    )
        idx = rng() * nRN();
    // variable starts:
    else
        idx = rng() * nRN() - numWalker;

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
    if(!save_histograms)
    {
        LOG(LOG_ERROR) << "the auxillary histogram information was not saved -> visualization not possible";
        return;
    }

    SpecWalker::svg(filename, with_hull);

    histograms[0].svg("histo0_" + filename);

    svg_histogram("histo_" + filename);
}

void ScentWalker::svg_histogram(const std::string filename) const
{
    if(!save_histograms)
    {
        LOG(LOG_ERROR) << "the auxillary histogram information was not saved -> visualization not possible";
        return;
    }

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
    if(!save_histograms)
    {
        LOG(LOG_ERROR) << "the auxillary histogram information was not saved -> visualization not possible";
        return;
    }

    GnuplotContour pic(filename);

    pic.data(histograms, starts);

    pic.save();
}
