#include "ScentWalker1D.hpp"

ScentWalker1D::ScentWalker1D(int d, int numSteps, int numWalker_in, int sideLength_in, int Tas_in, agent_start_t start_configuration_in, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia, bool save_histograms_in)
    : SpecWalker<int>(d, numSteps, rng, hull_algo, amnesia),
      numWalker(numWalker_in),
      sideLength(sideLength_in),
      Tas(Tas_in),
      relax(start_configuration_in == AS_RELAXED ? Tas_in : 0),
      periodic(false),
      start_configuration(start_configuration_in),
      save_histograms(save_histograms_in)
{
    assert(d == 1);
    assert(relax == 0);
    m_steps.resize(numSteps);

    updatedNumWalker();
    reconstruct();
}

void ScentWalker1D::updatedNumWalker()
{
    if(save_histograms)
        histograms = std::vector<HistogramND>(
            numWalker,
            HistogramND(sideLength, d, 0, sideLength)
        );

    pos = std::vector<Step<int>>(numWalker, Step<int>(d));
    step = std::vector<Step<int>>(numWalker, Step<int>(d));

    initial_trail = std::vector<std::vector<int>>(sideLength);
    for(int i=0; i<sideLength; ++i)
        initial_trail[i] = std::vector<int>(numWalker, -Tas-1);
}

void ScentWalker1D::reconstruct()
{
    starts.clear();
    pos.clear();
    step.clear();
    for(int i=0; i<sideLength; ++i)
        for(int j=0; j<numWalker; ++j)
            initial_trail[i][j] = -Tas-1;

    std::set<int> occupied_starts;
    int tmp_start;
    for(int j=0; j<numWalker; ++j)
    {
        tmp_start = rng() * sideLength;

        // test if this place is alreay occupied (low probability, but it happens)
        if(occupied_starts.find(tmp_start) != occupied_starts.end())
        {
            --j;
            continue;
        }
        occupied_starts.emplace(tmp_start);
        starts.emplace_back(std::vector<int>({tmp_start}));
        LOG(LOG_DEBUG) << "start " << j << ": " << starts[j];
    }

    if(!amnesia)
        random_numbers = rng.vector(numSteps*numWalker);

    init();
}

void ScentWalker1D::updateSteps()
{
    // use numWalker vectors for the steps (scent traces will be steps[now-Tas:now])
    // every walker has its own history
    // static -> will be allocated only once
    static World world(sideLength); // = std::vector<std::vector<int>(numWalker)>(sideLength);
    // FIXME this will allocate often and make the static useless
    for(int i=0; i<sideLength; ++i)
        world[i] = std::vector<int>(numWalker);
    // in the relaxed start configuration we have scent marks on the initial map
    if(start_configuration == AS_RELAXED)
    {
        world = initial_trail;
    }
    else
    {
        for(int i=0; i<sideLength; ++i)
            for(int j=0; j<numWalker; ++j)
                world[i][j] = -Tas-1;
    }

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
    for(int i=relax; i<numSteps+relax; ++i)
    {
        for(int j=0; j<numWalker; ++j)
        {
            bool marked_by_adversary = false;
            for(int k=0; k<numWalker; ++k)
            {
                if(k == j)
                    continue;
                if(i - world[pos[k].x()][j] < Tas)
                    marked_by_adversary = true;
            }

            if(marked_by_adversary)
            {
                step[j] = -step[j];
            }
            else
            {
                if(!amnesia)
                    step[j].fillFromRN(random_numbers[(i-relax)*numWalker + j]);
                else
                    step[j].fillFromRN(rng());
            }

            pos[j] += step[j];
            // periodic boundaries
            if(pos[j][0] < 0)
                pos[j][0] = sideLength - 1;
            if(pos[j][0] >= sideLength)
                pos[j][0] = 0;

            // mark new site
            world[pos[j].x()][j] = i;

            if(save_histograms)
                histograms[j].add(pos[j]);
            if(j == 0)
                m_steps[i-relax] = step[j];
        }
        // svg("a" + std::to_string(i));
    }
}

void ScentWalker1D::change(UniformRNG &rng, bool update)
{
    // I should do this in a far more clever way

    int idx;

    if(start_configuration == AS_RELAXED)
        idx = rng() * nRN();
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

void ScentWalker1D::undoChange()
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
