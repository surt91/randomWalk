#include "Simulation.hpp"

Simulation::Simulation(const Cmd &o, const bool fileOutput)
    : o(o),
      oss(o.data_path, std::ofstream::out),
      muted(false),
      fileOutput(fileOutput)
{
    start = clock();
    fails = 0;
    tries = 0;
    sum_L = 0.;
    sum_A = 0.;
    sum_r = 0.;
    sum_r2 = 0.;
    checksum = 0.;

    if(fileOutput)
        header(oss);

    S = prepareS(o);
}

Simulation::~Simulation()
{
    if(fileOutput)
        footer(oss);

    if(!muted)
    {
        LOG(LOG_INFO) << "# proposed changes: " << tries;
        LOG(LOG_INFO) << "# rejected changes: " << fails << " (" << (100.*fails / tries) << "%)";
        LOG(LOG_INFO) << "# mean L: " << (sum_L / o.iterations);
        LOG(LOG_INFO) << "# mean A: " << (sum_A / o.iterations);
        LOG(LOG_INFO) << "# mean r: " << (sum_r / o.iterations);
        LOG(LOG_INFO) << "# mean r2: " << (sum_r2 / o.iterations);
        LOG(LOG_INFO) << "# time/sweep in seconds: " << time_diff(start, clock(), o.iterations);
        LOG(LOG_INFO) << "# max vmem: " << vmPeak();
    }

    if(fileOutput)
        gzip(o.data_path);
}

void Simulation::prepare(std::unique_ptr<Walker>& w, const Cmd &o)
{
    UniformRNG rngReal(o.seedRealization);
    bool amnesia = false;
    if(o.sampling_method == SM_SIMPLESAMPLING)
        amnesia = true;

    if(o.type == WT_RANDOM_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new LatticeWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<LatticeWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
    }
    else if(o.type == WT_LOOP_ERASED_RANDOM_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<LoopErasedWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
    }
    else if(o.type == WT_SELF_AVOIDING_RANDOM_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new SelfAvoidingWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<SelfAvoidingWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
    }
    else if(o.type == WT_REAL_RANDOM_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new RealWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<RealWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
    }
    else if(o.type == WT_GAUSSIAN_RANDOM_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new GaussWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<GaussWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
    }
    else if(o.type == WT_LEVY_FLIGHT)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new LevyWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<LevyWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
    }
    else if(o.type == WT_CORRELATED_RANDOM_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new CorrelatedWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<CorrelatedWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
        w->setP1(o.mu);
        w->setP2(o.sigma);
    }
    else if(o.type == WT_ESCAPE_RANDOM_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new EscapeWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<EscapeWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
    }
    else if(o.type == WT_SCENT_RANDOM_WALK)
    {
        // If an image is requested, save information needed for visualisation
        bool visualize = !o.gp_path.empty() || !o.svg_path.empty();
        if(o.d == 1) {
            w = std::unique_ptr<Walker>(
                new ScentWalker1D(o.d, o.steps, o.numWalker, o.width, o.tas, o.agent_start, rngReal, o.chAlg, amnesia, visualize)
            );
        } else {
            w = std::unique_ptr<Walker>(
                new ScentWalker(o.d, o.steps, o.numWalker, o.width, o.tas, o.agent_start, rngReal, o.chAlg, amnesia, visualize)
            );
        }
    }
    else if(o.type == WT_TRUE_SELF_AVOIDING_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new TrueSelfAvoidingWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<TrueSelfAvoidingWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
        w->setP1(o.beta);
    }
    else if(o.type == WT_RESET_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new ResetWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<ResetWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
        w->setP1(o.resetrate);
    }
    else if(o.type == WT_BRANCH_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new BranchingGauss(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<BranchingGauss>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
        w->setP1(o.resetrate);
    }
    else if(o.type == WT_RUNANDTUMBLE_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new RunAndTumbleWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<RunAndTumbleWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
        w->setP1(o.gamma);
    }
    else if(o.type == WT_RUNANDTUMBLE_T_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new RunAndTumbleWalkerT(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<RunAndTumbleWalkerT>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
        w->setP1(o.gamma);
        w->setP2(o.total_length);
    }
    else if(o.type == WT_RETURNING_LATTICE_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new ReturningLatticeWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<ReturningLatticeWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
    }
    else if(o.type == WT_GAUSSIAN_RESET_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new GaussResetWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<GaussResetWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
        w->setP1(o.resetrate);
    }
    else if(o.type == WT_BROWNIAN_RESET_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new BrownianResetWalker(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<BrownianResetWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
        w->setP1(o.resetrate);
        w->setP2(o.total_length);
    }
    else if(o.type == WT_BROWNIAN_RESET_WALK_SHIFTED)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new BrownianResetWalkerShifted(o.d, o.steps, rngReal, o.chAlg, amnesia));
        else
            w = std::unique_ptr<Walker>(
                new MultipleWalker<BrownianResetWalkerShifted>(o.d, o.steps, o.numWalker, rngReal, o.chAlg, amnesia)
            );
        w->setP1(o.resetrate);
        w->setP2(o.total_length);
        w->setP3(o.shift);
    }
    else
    {
        LOG(LOG_ERROR) << "type " << o.type << " is not known";
    }

    w->updateHull();
}

std::function<double(const std::unique_ptr<Walker>&)> Simulation::prepareS(const Cmd &o)
{
    std::function<double(const std::unique_ptr<Walker>&)> S;

    if(o.wantedObservable == WO_SURFACE_AREA)
        S = [](const std::unique_ptr<Walker> &w){ return w->L(); };
    else if(o.wantedObservable == WO_VOLUME)
        S = [](const std::unique_ptr<Walker> &w){ return w->A(); };
    else if(o.wantedObservable == WO_PASSAGE)
    {
        int t1 = o.passageTimeStart;
        S = [t1](const std::unique_ptr<Walker> &w){ return w->passage(t1); };
    }
    else
        LOG(LOG_ERROR) << "observable " << o.wantedObservable << " is not known";

    return S;
}

/** Estimate an upper bound with a greedy heuristic */
double Simulation::getReasonalbleUpperBound(Cmd &o)
{
    std::unique_ptr<Walker> w;
    prepare(w, o);
    auto S = prepareS(o);

    w->goDownhill(true, o.wantedObservable,100);
    w->svg("r_upper.svg");

    return S(w);
}

/** Estimate a lower bound with a greedy heuristic */
double Simulation::getReasonalbleLowerBound(Cmd &o)
{
    std::unique_ptr<Walker> w;
    prepare(w, o);
    auto S = prepareS(o);

    w->goDownhill(false, o.wantedObservable, 100);
    w->svg("r_lower.svg");

    return S(w);
}

/** get the exact upper bound (possibly infinity) */
double Simulation::getUpperBound(Cmd &o)
{
    double S_max = 0;

    std::unique_ptr<Walker> w;
    prepare(w, o);
    auto S = prepareS(o);

    // the degenerate case is -- hopefully -- the case of maximum Volume
    if(o.wantedObservable == WO_VOLUME)
    {
        w->degenerateMaxVolume();
        S_max = S(w);
    }
    else
    {
        w->degenerateMaxSurface();
        S_max = S(w);
    }

    w->svg("upper.svg");

    return S_max;
}

/** get the exact lower bound (possibly zero) */
double Simulation::getLowerBound(Cmd &o)
{
    double S_min = 0;

    std::unique_ptr<Walker> w;
    prepare(w, o);
    auto S = prepareS(o);

    if(o.wantedObservable == WO_VOLUME)
    {
        w->degenerateMinVolume();
        S_min = S(w);
    }
    else
    {
        w->degenerateMinSurface();
        S_min = S(w);
    }

    w->svg("lower.svg");

    return S_min;
}

void Simulation::header(std::ofstream &oss)
{

    // test, if we can write to the file
    if(!oss.good())
    {
        LOG(LOG_ERROR) << "Path is not writable '" << o.data_path << "'";
        //~ throw std::invalid_argument("Path is not writable");
    }
    else
    {
        // set output to 12 significant digits.
        // storage is cheap and it can not worsen the results
        oss.precision(12);
        // save the commandline invocation to the outputfile
        oss << "# " << o.text << "\n";
        oss << "# Version: " << VERSION << "\n";
        oss << "# Compiled: " << __DATE__ << " " << __TIME__ << "\n";

        time_t _tm = time(NULL);
        struct tm *curtime = localtime(&_tm);
        oss << "# Started: " << asctime(curtime) << "\n";
    }
}

void Simulation::footer(std::ofstream &oss)
{
    if(!oss.good())
    {
        LOG(LOG_ERROR) << "Path is not writable '" << o.data_path << "'";
        //~ throw std::invalid_argument("Path is not writable");
    }
    else
    {
        // save runtime statistics
        oss << "# proposed changes: " << tries << "\n";
        oss << "# rejected changes: " << fails << " (" << (100.*fails / tries) << "%)" << "\n";
        // time will be overestimated because of the equilibration
        time_t _tm = time(NULL);
        struct tm *curtime = localtime(&_tm);
        oss << "# Ended: " << asctime(curtime) << "\n";
        oss << "# time/sweep in seconds: " << time_diff(start, clock(), o.iterations) << "\n";
        oss << "# max vmem: " << vmPeak() << "\n";
    }
    oss.close();
}

void Simulation::write_observables(std::unique_ptr<Walker> &w, int i, std::ofstream &oss)
{
    if(!oss.good())
    {
        LOG(LOG_ERROR) << "Path is not writable '" << o.data_path << "'";
        //~ throw std::invalid_argument("Path is not writable");
    }
    else
    {
        oss << i << " "
            << w->L() << " "
            << w->A() << " ";

        if(w->d > 1)
        {
            oss << w->r() << " "
                << w->r2() << " "
                << w->maxDiameter() << " "
                << w->rx() << " "
                << w->ry() << " "
                << w->num_on_hull() << " "
                << w->oblateness() << " "
                << w->visitedSites() << " "
                << w->enclosedSites() << " "
                << w->length() << " "
                << w->steps_taken() << " "
                << w->argminx() << " "
                << w->argmaxx() << " "
                << w->minx() << " "
                << w->maxx() << " "
                << w->num_resets() << " "
                << w->maxsteps_partialwalk() << " "
                << w->maxlen_partialwalk() << " ";
        }
    }
}
