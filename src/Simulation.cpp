#include "Simulation.hpp"

Simulation::Simulation(const Cmd &o)
    : o(o),
      oss(o.data_path, std::ofstream::out)
{
    begin = clock();
    fail = 0;
    trys = 0;

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
    LOG(LOG_INFO) << "# rejected changes: " << fail << " (" << (100.*fail / trys) << "%)";
    LOG(LOG_INFO) << "# time in seconds: " << time_diff(begin, clock());
    LOG(LOG_INFO) << "# max vmem: " << vmPeak();

    // save runtime statistics
    oss << "# rejected changes: " << fail << " (" << (100.*fail / trys) << "%)" << "\n";
    oss << "# time in seconds: " << time_diff(begin, clock()) << "\n";
    oss << "# max vmem: " << vmPeak() << std::endl;

    std::string cmd("gzip -f ");
    system((cmd+o.data_path).c_str());
}

void Simulation::prepare(std::unique_ptr<Walker>& w)
{
    // do use different seeds, if using openmp
    UniformRNG rngReal(o.seedRealization * (omp_get_thread_num()+1));

    if(o.type == WT_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new LatticeWalker(o.d, o.steps, rngReal, o.chAlg));
    else if(o.type == WT_LOOP_ERASED_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, o.steps, rngReal, o.chAlg));
    else if(o.type == WT_SELF_AVOIDING_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new SelfAvoidingWalker(o.d, o.steps, rngReal, o.chAlg));
    else
        LOG(LOG_ERROR) << "type " << o.type << " is not known";

    w->updateHull();
}
