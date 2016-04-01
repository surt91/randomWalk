#include "LargeDeviation.hpp"

int equilibrate(const Cmd &o, std::unique_ptr<Walker>& w1, UniformRNG& rngMC1, std::function<double(std::unique_ptr<Walker>&)> S)
{
    int t_eq = 0;
    RollingMean rmean1(100), rmean2(100);

    std::unique_ptr<Walker> w2;
    UniformRNG rngReal(o.seedRealization);
    UniformRNG rngMC(rngReal() * INT_MAX);
    std::ofstream oss("equilibration.dat", std::ofstream::out);

    if(o.type == WT_RANDOM_WALK)
    {
        //~ w1 = std::unique_ptr<Walker>(new Walker(o.d, o.steps, rngReal, o.chAlg));
        w2 = std::unique_ptr<Walker>(new Walker(o.d, o.steps, rngReal, o.chAlg));
    }
    else if(o.type == WT_LOOP_ERASED_RANDOM_WALK)
    {
        //~ w1 = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, o.steps, rngReal, o.chAlg));
        w2 = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, o.steps, rngReal, o.chAlg));
    }
    w2->degenerate();

    // FIXME: this is edundant code, needs to be cleaned up
    while(true)
    {
        // one sweep, i.e., one change try for each site
        for(int j=0; j<o.steps; ++j)
        {
            int rn_to_change1 = o.steps * rngMC1();
            int rn_to_change2 = o.steps * rngMC();

            // change one random number to another random number
            // save the random number before the change
            double oldS1 = S(w1);
            double oldS2 = S(w2);
            double oldRN1 = w1->rnChange(rn_to_change1, rngMC1());
            double oldRN2 = w2->rnChange(rn_to_change2, rngMC());

            // Metropolis rejection
            double p_acc1 = std::min({1.0, exp(-(S(w1) - oldS1)/o.theta)});
            double p_acc2 = std::min({1.0, exp(-(S(w2) - oldS2)/o.theta)});
            if(p_acc1 < 1 - rngMC1())
                w1->rnChange(rn_to_change1, oldRN1);
            if(p_acc2 < 1 - rngMC())
                w2->rnChange(rn_to_change2, oldRN2);
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

void run(const Cmd &o)
{
    clock_t begin = clock();
    int fail = 0;
    int t_eq = 0;
    int t_corr = 10; // just a guess
    UniformRNG rngReal(o.seedRealization);
    UniformRNG rngMC(o.seedMC);
    std::ofstream oss(o.data_path, std::ofstream::out);
    if(!oss.good())
    {
        LOG(LOG_ERROR) << "Path is not writable " << o.data_path;
        throw std::invalid_argument("Path is not writable");
    }
    oss << "# large deviation simulation at theta=" << o.theta << " and steps=" << o.steps << "\n";
    oss << "# sweeps L A\n";

    std::unique_ptr<Walker> w;
    // does not work for loop erased yet
    if(o.type == WT_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new Walker(o.d, o.steps, rngReal, o.chAlg));
    else if(o.type == WT_LOOP_ERASED_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new LoopErasedWalker(o.d, o.steps, rngReal, o.chAlg));
    else if(o.type == WT_SELF_AVOIDING_RANDOM_WALK)
        w = std::unique_ptr<Walker>(new SelfAvoidingWalker(o.d, o.steps, rngReal, o.chAlg));
    w->convexHull();

    w->saveConfiguration(o.conf_path, false);

    std::function<double(std::unique_ptr<Walker>&)> S;
    if(o.wantedObservable == WO_SURFACE_AREA)
        S = [](std::unique_ptr<Walker> &w){ return w->L(); };
    else if(o.wantedObservable == WO_VOLUME)
        S = [](std::unique_ptr<Walker> &w){ return w->A(); };

    t_eq = equilibrate(o, w, rngMC, S);

    for(int i=t_eq; i<o.iterations+2*t_eq; ++i)
    {
        // one sweep, i.e., one change try for each site
        for(int j=0; j<o.steps; ++j)
        {
            int rn_to_change = o.steps * rngMC();

            // change one random number to another random number
            // save the random number before the change
            double oldS = S(w);
            double oldRN = w->rnChange(rn_to_change, rngMC());

            // Metropolis rejection
            double p_acc = std::min({1.0, exp(-(S(w) - oldS)/o.theta)});
            if(p_acc < 1 - rngMC())
            {
                ++fail;
                w->rnChange(rn_to_change, oldRN);
            }
        }
        // TODO: only save after t_eq, and only statisically independent configurations
        if(i >= 2*t_eq && (i-2*t_eq) % t_corr == 0)
        {
            w->saveConfiguration(o.conf_path);

            LOG(LOG_TOO_MUCH) << "Area  : " << w->L();
            LOG(LOG_TOO_MUCH) << "Volume: " << w->A();
            LOG(LOG_DEBUG) << "Iteration: " << i;
            oss << i << " " << w->L() << " " << w->A() << std::endl;
        }
    }

    LOG(LOG_INFO) << "# rejected changes: " << fail << " (" << (100. * fail/(o.iterations*o.steps)) << "%)";
    LOG(LOG_INFO) << "# time in seconds: " << time_diff(begin, clock());
    LOG(LOG_INFO) << "# max vmem: " << vmPeak();

    // save runtime statistics
    oss << "# rejected changes: " << fail << " (" << (100. * fail/(o.iterations*o.steps)) << "%)" << "\n";
    oss << "# time in seconds: " << time_diff(begin, clock()) << "\n";
    oss << "# max vmem: " << vmPeak() << std::endl;

    std::string cmd("gzip -f ");
    system((cmd+o.data_path).c_str());
    system((cmd+o.conf_path).c_str());

    if(!o.svg_path.empty())
        w->svg(o.svg_path, true);

    if(!o.pov_path.empty())
        w->pov(o.pov_path, true);
}
