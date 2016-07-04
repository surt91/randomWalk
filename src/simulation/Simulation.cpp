#include "Simulation.hpp"

Simulation::Simulation(const Cmd &o)
    : o(o),
      oss(o.data_path, std::ofstream::out),
      muted(false)
{
    begin = clock();
    fails = 0;
    tries = 0;
    sum_L = 0.;
    sum_A = 0.;
    sum_r = 0.;
    sum_r2 = 0.;

    header(oss);

    S = prepareS(o);
}

Simulation::~Simulation()
{
    footer(oss);

    if(!muted)
    {
        LOG(LOG_INFO) << "# proposed changes: " << tries;
        LOG(LOG_INFO) << "# rejected changes: " << fails << " (" << (100.*fails / tries) << "%)";
        LOG(LOG_INFO) << "# mean L: " << (sum_L / o.iterations);
        LOG(LOG_INFO) << "# mean A: " << (sum_A / o.iterations);
        LOG(LOG_INFO) << "# mean r: " << (sum_r / o.iterations);
        LOG(LOG_INFO) << "# mean r2: " << (sum_r2 / o.iterations);
        LOG(LOG_INFO) << "# time/sweep in seconds: " << time_diff(begin, clock(), o.iterations);
        LOG(LOG_INFO) << "# max vmem: " << vmPeak();
    }

    std::string cmd("gzip -f ");
    system((cmd+o.data_path).c_str());
}

void Simulation::prepare(std::unique_ptr<Walker>& w, const Cmd &o)
{
    // do use different seeds, if using openmp
    UniformRNG rngReal(o.seedRealization * (omp_get_thread_num()+1));

    if(o.type == WT_RANDOM_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new LatticeWalker(o.d, o.steps, rngReal, o.chAlg));
        else
            w = std::unique_ptr<Walker>(new MultipleWalker<LatticeWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg));
    }
    else if(o.type == WT_LOOP_ERASED_RANDOM_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, o.steps, rngReal, o.chAlg));
        else
            w = std::unique_ptr<Walker>(new MultipleWalker<LoopErasedWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg));
    }
    else if(o.type == WT_SELF_AVOIDING_RANDOM_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new SelfAvoidingWalker(o.d, o.steps, rngReal, o.chAlg));
        else
            w = std::unique_ptr<Walker>(new MultipleWalker<SelfAvoidingWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg));
    }
    else if(o.type == WT_REAL_RANDOM_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new RealWalker(o.d, o.steps, rngReal, o.chAlg));
        else
            w = std::unique_ptr<Walker>(new MultipleWalker<RealWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg));
    }
    else if(o.type == WT_GAUSSIAN_RANDOM_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new GaussWalker(o.d, o.steps, rngReal, o.chAlg));
        else
            w = std::unique_ptr<Walker>(new MultipleWalker<GaussWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg));
    }
    else if(o.type == WT_LEVY_FLIGHT)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new LevyWalker(o.d, o.steps, rngReal, o.chAlg));
        else
            w = std::unique_ptr<Walker>(new MultipleWalker<LevyWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg));
    }
    else if(o.type == WT_CORRELATED_RANDOM_WALK)
    {
        if(o.numWalker == 1)
            w = std::unique_ptr<Walker>(new CorrelatedWalker(o.d, o.steps, rngReal, o.chAlg));
        else
            w = std::unique_ptr<Walker>(new MultipleWalker<CorrelatedWalker>(o.d, o.steps, o.numWalker, rngReal, o.chAlg));
        w->setP1(o.mu);
        w->setP2(o.sigma);
    }
    else
    {
        LOG(LOG_ERROR) << "type " << o.type << " is not known";
    }

    w->updateHull();
}

std::function<double(std::unique_ptr<Walker>&)> Simulation::prepareS(const Cmd &o)
{
    std::function<double(std::unique_ptr<Walker>&)> S;

    if(o.wantedObservable == WO_SURFACE_AREA)
        S = [](const std::unique_ptr<Walker> &w){ return w->L(); };
    else if(o.wantedObservable == WO_VOLUME)
        S = [](const std::unique_ptr<Walker> &w){ return w->A(); };
    else
        LOG(LOG_ERROR) << "observable " << o.wantedObservable << " is not known";

    return S;
}

double Simulation::getUpperBound(Cmd &o)
{
    double S_min = 0;

    std::unique_ptr<Walker> w;
    prepare(w, o);
    auto S = prepareS(o);

    // the degenerate case is -- hopefully -- the case of maximum Volume
    if(o.wantedObservable == WO_VOLUME)
    {
        w->degenerateMaxVolume();
        S_min = S(w);
    }
    else
    {
        w->degenerateMaxSurface();
        S_min = S(w);
    }

    w->svg("upper.svg");

    return S_min;
}

double Simulation::getLowerBound(Cmd &o)
{
    if(o.wantedObservable == WO_VOLUME)
        return 0;

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
    // set output to 12 significant digits.
    // storage is cheap and it can not worsen the results
    oss.precision(12);
    // save the commandline invocation to the outputfile
    oss << "# " << o.text << "\n";
    oss << "# Version: " << VERSION << "\n";
    oss << "# Compiled: " << __DATE__ << " " << __TIME__ << std::endl;

    time_t _tm = time(NULL);
    struct tm *curtime = localtime(&_tm);
    oss << "# Started: " << asctime(curtime);

    if(!oss.good())
    {
        LOG(LOG_ERROR) << "Path is not writable " << o.data_path;
        throw std::invalid_argument("Path is not writable");
    }
}

void Simulation::footer(std::ofstream &oss)
{
    // save runtime statistics
    oss << "# proposed changes: " << tries << "\n";
    oss << "# rejected changes: " << fails << " (" << (100.*fails / tries) << "%)" << "\n";
    // time will be overestimated because of the equilibration
    time_t _tm = time(NULL);
    struct tm *curtime = localtime(&_tm);
    oss << "# Ended: " << asctime(curtime) << std::endl;
    oss << "# time/sweep in seconds: " << time_diff(begin, clock(), o.iterations) << "\n";
    oss << "# max vmem: " << vmPeak() << "\n";
    oss.close();
}
