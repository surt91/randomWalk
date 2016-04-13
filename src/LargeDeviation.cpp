#include "LargeDeviation.hpp"

void prepare(const Cmd &o, std::unique_ptr<Walker>& w, std::function<double(std::unique_ptr<Walker>&)>& S)
{
    // do use different seeds, if using openmp
    UniformRNG rngReal(o.seedRealization * (omp_get_thread_num()+1));

    if(o.type == WT_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new Walker(o.d, o.steps, rngReal, o.chAlg));
    else if(o.type == WT_LOOP_ERASED_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, o.steps, rngReal, o.chAlg));
    else if(o.type == WT_SELF_AVOIDING_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new SelfAvoidingWalker(o.d, o.steps, rngReal, o.chAlg));
    else
        LOG(LOG_ERROR) << "type " << o.type << " is not known to 'metropolis'";
    w->convexHull();

    if(o.wantedObservable == WO_SURFACE_AREA)
        S = [](const std::unique_ptr<Walker> &w){ return w->L(); };
    else if(o.wantedObservable == WO_VOLUME)
        S = [](const std::unique_ptr<Walker> &w){ return w->A(); };
    else
        LOG(LOG_ERROR) << "observalbe " << o.wantedObservable << " is not known";
}

double getUpperBound(const Cmd &o)
{
    double S_min = 0;

    std::unique_ptr<Walker> w;
    std::function<double(std::unique_ptr<Walker>&)> S;
    prepare(o, w, S);

    // the degenerate case is -- hopefully -- the case of maximum Volume
    if(o.wantedObservable == WO_VOLUME)
    {
        w->degenerate();
        S_min = S(w);
    }
    else
    {
        w->degenerateMaxSurface();
        S_min = S(w);
    }

    return S_min;
}

double getLowerBound(const Cmd &o)
{
    if(o.wantedObservable == WO_VOLUME)
        return 0;

    double S_min = 0;
    if(o.type == WT_RANDOM_WALK)
        S_min = 2;
    else
        S_min = 4*sqrt(o.steps);

    return S_min;
}

int equilibrate(const Cmd &o, std::unique_ptr<Walker>& w1, UniformRNG& rngMC1, std::function<double(std::unique_ptr<Walker>&)> S)
{
    int t_eq = 0;
    RollingMean rmean1(100), rmean2(100);

    std::unique_ptr<Walker> w2;
    UniformRNG rngReal(o.seedRealization);
    UniformRNG rngMC(rngReal() * INT_MAX);
    std::ofstream oss("equilibration.dat", std::ofstream::out);

    if(o.type == WT_RANDOM_WALK)
        w2 = std::unique_ptr<Walker>(new Walker(o.d, o.steps, rngReal, o.chAlg));
    else if(o.type == WT_LOOP_ERASED_RANDOM_WALK)
        w2 = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, o.steps, rngReal, o.chAlg));
    else if(o.type == WT_SELF_AVOIDING_RANDOM_WALK)
        w2 = std::unique_ptr<Walker>(new SelfAvoidingWalker(o.d, o.steps, rngReal, o.chAlg));
    else
        LOG(LOG_ERROR) << "type " << o.type << " is not known to 'equilibrate'";
    w2->degenerate();

    // FIXME: this is edundant code, needs to be cleaned up
    while(true)
    {
        // one sweep, i.e., one change try for each site
        for(int j=0; j<o.steps; ++j)
        {
            // change one random number to another random number
            // save the random number before the change
            double oldS1 = S(w1);
            double oldS2 = S(w2);
            w1->change(rngMC1);
            w2->change(rngMC);

            if(!o.simpleSampling)
            {
                // Metropolis rejection
                double p_acc1 = std::min({1.0, exp(-(S(w1) - oldS1)/o.theta)});
                double p_acc2 = std::min({1.0, exp(-(S(w2) - oldS2)/o.theta)});
                if(p_acc1 < 1 - rngMC1())
                    w1->undoChange();
                if(p_acc2 < 1 - rngMC())
                    w2->undoChange();
            }
        }
        oss << t_eq << " " << w1->L() << " " << w2->L() << " " << w1->A() << " " << w2->A() << std::endl;

        // fluctuation within 1%
        if(std::abs(rmean1.add(S(w1))/rmean2.add(S(w2)) - 1) < 0.01)
            break;
        ++t_eq;
    }

    LOG(LOG_INFO) << "Equilibration estimate: t_eq = " << t_eq;
    LOG(LOG_INFO) << "plot with gnuplot: p \"equilibration.dat\" u 1:4 w l, \"\" u 1:5 w l";
    return t_eq;
}

void metropolis(const Cmd &o)
{
    clock_t begin = clock();
    int fail = 0;
    int t_eq = 0;
    UniformRNG rngMC(o.seedMC);
    std::ofstream oss(o.data_path, std::ofstream::out);
    if(!oss.good())
    {
        LOG(LOG_ERROR) << "Path is not writable " << o.data_path;
        throw std::invalid_argument("Path is not writable");
    }
    if(!o.simpleSampling)
    {
        oss << "# large deviation simulation at theta=" << o.theta << " and steps=" << o.steps << "\n";
    }
    else
    {
        oss << "# simple sampling simulation with steps=" << o.steps << "\n";
    }
    oss << "# sweeps L A\n";

    std::unique_ptr<Walker> w;
    std::function<double(std::unique_ptr<Walker>&)> S;
    prepare(o, w, S);

    t_eq = equilibrate(o, w, rngMC, S);

    for(int i=t_eq; i<o.iterations+2*t_eq; ++i)
    {
        // one sweep, i.e., one change try for each site
        for(int j=0; j<o.steps; ++j)
        {
            // change one random number to another random number
            double oldS = S(w);
            w->change(rngMC);

            if(!o.simpleSampling)
            {
                // Metropolis rejection
                //~ double p_acc = std::min({1.0, exp(-(S(w) - oldS)/o.theta)});
                double p_acc = exp((oldS - S(w))/o.theta);
                if(p_acc < rngMC())
                {
                    ++fail;
                    w->undoChange();
                }
            }
        }
        // TODO: only save after t_eq
        if(i >= 2*t_eq)
        {
            if(!o.conf_path.empty())
                w->saveConfiguration(o.conf_path);

            LOG(LOG_TOO_MUCH) << "Area  : " << w->L();
            LOG(LOG_TOO_MUCH) << "Volume: " << w->A();
            LOG(LOG_DEBUG) << "Iteration: " << i;
            oss << i << " " << w->L() << " " << w->A() << std::endl;

            //~ w->svg("svg/eq_" + std::to_string(i) + "_1.svg", true);
            //~ w->pov("svg/eq_" + std::to_string(i) + "_1.pov", true);
        }
    }

    LOG(LOG_INFO) << "# rejected changes: " << fail << " (" << (100. * fail/((o.iterations+t_eq)*o.steps)) << "%)";
    LOG(LOG_INFO) << "# time in seconds: " << time_diff(begin, clock());
    LOG(LOG_INFO) << "# max vmem: " << vmPeak();

    // save runtime statistics
    oss << "# rejected changes: " << fail << " (" << (100. * fail/(o.iterations*o.steps)) << "%)" << "\n";
    oss << "# time in seconds: " << time_diff(begin, clock()) << "\n";
    oss << "# max vmem: " << vmPeak() << std::endl;

    std::string cmd("gzip -f ");
    system((cmd+o.data_path).c_str());
    if(!o.conf_path.empty())
        system((cmd+o.conf_path).c_str());

    if(!o.svg_path.empty())
        w->svg(o.svg_path, true);

    if(!o.pov_path.empty())
        w->pov(o.pov_path, true);
}

// 10.1103/PhysRevLett.86.2050 (original paper)
// http://cdn.intechopen.com/pdfs-wm/14019.pdf (implementations hints)
void wang_landau(const Cmd &o)
{
    // parameters of the algorithm
    const double lnf_min = 1e-8;
    const double flatness_criterion = 0.8;

    clock_t begin = clock();
    std::ofstream oss(o.data_path, std::ofstream::out);
    if(!oss.good())
    {
        LOG(LOG_ERROR) << "Path is not writable " << o.data_path;
        throw std::invalid_argument("Path is not writable");
    }

    // Histogram and DensityOfStates need the same binning ... probably
    const double lb = getLowerBound(o);
    const double ub = getUpperBound(o) + 1;

    // do not make a bin for every integer, since not every integer is possible
    const int bins = (ub-lb)/3;
    LOG(LOG_DEBUG) << lb << " " << ub << " " << bins;

    Histogram H(bins, lb, ub);
    DensityOfStates g(bins, lb, ub);

    oss << "# First line are the centers of the bins\n";
    oss << "# Following lines are unnormalized, log densities of the bin\n";
    oss << "# one line per iteration of the wang landau algorithm\n";
    oss << g.binCentersString() << std::endl;

    // run in parallel,
    // use dynamic schedule, since single iterations may need strongly fluctuating time
    #pragma omp parallel firstprivate(H, g) shared(oss)
    {
        // rngs should be local to the threads, with different seeds
        // FIXME: think about a better seed
        UniformRNG rngMC(o.seedMC * (omp_get_thread_num()+1));

        std::unique_ptr<Walker> w;
        std::function<double(std::unique_ptr<Walker>&)> S;
        prepare(o, w, S);

        #pragma omp for schedule(dynamic)
        for(int i=0; i<o.iterations; ++i)
        {
            double lnf = 1;
            while(lnf > lnf_min)
            {
                LOG(LOG_TOO_MUCH) << "t" << omp_get_thread_num() << " : ln f " << lnf;
                do
                {
                    double oldS = S(w);
                    w->change(rngMC);
                    double p_acc = exp(g[oldS] - g[S(w)]);
                    if(p_acc < rngMC())
                        w->undoChange();

                    // the paper is not clear whether this is done only if the
                    // change is accepted, but I assume always, as usual for MC
                    g[S(w)] += lnf;
                    H.add(S(w));
                } while(H.min() < flatness_criterion * H.mean());
                // run until the histogram is flat and we have a few samples
                H.reset();
                lnf /= 2;
            }
            // save g to file
            #pragma omp critical
            oss << g.dataString() << std::endl;

            g.reset();
        }
    }

    LOG(LOG_INFO) << "# time in seconds: " << time_diff(begin, clock());
    LOG(LOG_INFO) << "# max vmem: " << vmPeak();

    // save runtime statistics
    oss << "# time in seconds: " << time_diff(begin, clock()) << "\n";
    oss << "# max vmem: " << vmPeak() << std::endl;

    std::string cmd("gzip -f ");
    system((cmd+o.data_path).c_str());

    //~ if(!o.svg_path.empty())
        //~ w->svg(o.svg_path, true);

    //~ if(!o.pov_path.empty())
        //~ w->pov(o.pov_path, true);
}
