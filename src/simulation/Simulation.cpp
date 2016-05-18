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

    // save the commandline invocation to the outputfile
    oss << "# " << o.text << "\n";

    if(!oss.good())
    {
        LOG(LOG_ERROR) << "Path is not writable " << o.data_path;
        throw std::invalid_argument("Path is not writable");
    }

    if(o.wantedObservable == WO_SURFACE_AREA)
        S = [](const std::unique_ptr<Walker> &w){ return w->L(); };
    else if(o.wantedObservable == WO_VOLUME)
        S = [](const std::unique_ptr<Walker> &w){ return w->A(); };
    else
        LOG(LOG_ERROR) << "observable " << o.wantedObservable << " is not known";
}

Simulation::~Simulation()
{
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

    // save runtime statistics
    oss << "# proposed changes: " << tries << "\n";
    oss << "# rejected changes: " << fails << " (" << (100.*fails / tries) << "%)" << "\n";
    // time will be overestimated because of the equilibration
    oss << "# time/sweep in seconds: " << time_diff(begin, clock(), o.iterations) << "\n";
    oss << "# max vmem: " << vmPeak() << std::endl;
    oss.close();

    std::string cmd("gzip -f ");
    system((cmd+o.data_path).c_str());
}

void Simulation::prepare(std::unique_ptr<Walker>& w, const Cmd &o)
{
    // do use different seeds, if using openmp
    UniformRNG rngReal(o.seedRealization * (omp_get_thread_num()+1));

    if(o.type == WT_RANDOM_WALK)
    {
        w = std::unique_ptr<Walker>(new LatticeWalker(o.d, o.steps, rngReal, o.chAlg));
    }
    else if(o.type == WT_LOOP_ERASED_RANDOM_WALK)
    {
        w = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, o.steps, rngReal, o.chAlg));
    }
    else if(o.type == WT_SELF_AVOIDING_RANDOM_WALK)
    {
        w = std::unique_ptr<Walker>(new SelfAvoidingWalker(o.d, o.steps, rngReal, o.chAlg));
    }
    else if(o.type == WT_REAL_RANDOM_WALK)
    {
        w = std::unique_ptr<Walker>(new RealWalker(o.d, o.steps, rngReal, o.chAlg));
    }
    else if(o.type == WT_GAUSSIAN_RANDOM_WALK)
    {
        w = std::unique_ptr<Walker>(new GaussWalker(o.d, o.steps, rngReal, o.chAlg));
    }
    else if(o.type == WT_LEVY_FLIGHT)
    {
        w = std::unique_ptr<Walker>(new LevyWalker(o.d, o.steps, rngReal, o.chAlg));
    }
    else if(o.type == WT_CORRELATED_RANDOM_WALK)
    {
        w = std::unique_ptr<Walker>(new CorrelatedWalker(o.d, o.steps, rngReal, o.chAlg));
        w->setP1(o.mu);
        w->setP2(o.sigma);
    }
    else
    {
        LOG(LOG_ERROR) << "type " << o.type << " is not known";
    }

    w->updateHull();
}
