#include "Metropolis.hpp"

Metropolis::Metropolis(const Cmd &o)
    : Simulation(o)
{
}

int Metropolis::equilibrate(std::unique_ptr<Walker>& w1, UniformRNG& rngMC1)
{
    int t_eq = 0;
    RollingMean rmean1(100), rmean2(100);

    UniformRNG rngMC(o.seedMC + 1);

    if(Logging::verbosity >= LOG_DEBUG)
        std::ofstream oss("equilibration.dat", std::ofstream::out);

    std::unique_ptr<Walker> w2;
    prepare(w2);
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

        if(Logging::verbosity >= LOG_DEBUG)
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

void Metropolis::run()
{
    int fail = 0;
    int t_eq = 0;
    UniformRNG rngMC(o.seedMC);

    if(!o.simpleSampling)
        oss << "# large deviation simulation at theta=" << o.theta << " and steps=" << o.steps << "\n";
    else
        oss << "# simple sampling simulation with steps=" << o.steps << "\n";
    oss << "# sweeps L A\n";

    std::unique_ptr<Walker> w;
    prepare(w);

    t_eq = equilibrate(w, rngMC);

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
                double p_acc = exp((oldS - S(w))/o.theta);
                if(p_acc < rngMC())
                {
                    ++fail;
                    w->undoChange();
                }
            }
        }
        if(i >= 2*t_eq)
        {
            if(!o.conf_path.empty())
                w->saveConfiguration(o.conf_path);

            LOG(LOG_TOO_MUCH) << "Area  : " << w->L();
            LOG(LOG_TOO_MUCH) << "Volume: " << w->A();
            LOG(LOG_DEBUG) << "Iteration: " << i;
            oss << i << " " << w->L() << " " << w->A() << std::endl;
        }
    }

    LOG(LOG_INFO) << "# rejected changes: " << fail << " (" << (100. * fail/((o.iterations+t_eq)*o.steps)) << "%)";

    // save runtime statistics
    oss << "# rejected changes: " << fail << " (" << (100. * fail/(o.iterations*o.steps)) << "%)" << "\n";
}
