#include "Metropolis.hpp"

Metropolis::Metropolis(const Cmd &o)
    : Simulation(o)
{
}

/** Equilibrate the walk
 *
 * Detects if equilibration is reached by comparing rolling means of
 * different starting configurations.
 *
 * \param w1 the walk which should be equilibrated
 * \param rngMC1 the random number generator used for w1
 * \return the equilibration time
 */
int Metropolis::equilibrate(std::unique_ptr<Walker>& w1, UniformRNG& rngMC1)
{
    int t_eq = 0;
    const int samples = 100;
    const double threshold = 0.05; // 5% threshold
    bool start = false;

    RollingMean rmean1(samples);
    RollingMean rmean2(samples);
    //~ RollingMean rmean3(samples);
    //~ RollingMean rmean4(samples);
    //~ RollingMean rmean5(samples);

    UniformRNG rngMC((o.seedMC+1)*0x243F6A8885A3);

    std::ofstream oss("equilibration.dat", std::ofstream::out);

    std::unique_ptr<Walker> w2;
    prepare(w2);
    w2->degenerateMaxVolume();

    //~ std::unique_ptr<Walker> w3;
    //~ prepare(w3);
    //~ w3->degenerateMaxSurface();

    //~ std::unique_ptr<Walker> w4;
    //~ prepare(w4);
    //~ w4->degenerateSpiral();

    //~ std::unique_ptr<Walker> w5;
    //~ prepare(w5);
    //~ w5->degenerateStraight();

    // FIXME: this is redundant code, needs to be cleaned up
    while(true)
    {
        // one sweep, i.e., one change try for each site
        for(int j=0; j<o.steps; ++j)
        {
            // change one random number to another random number
            // save the random number before the change
            double oldS1 = S(w1);
            double oldS2 = S(w2);
            //~ double oldS3 = S(w3);
            //~ double oldS4 = S(w4);
            //~ double oldS5 = S(w5);
            w1->change(rngMC1);
            w2->change(rngMC);
            //~ w3->change(rngMC);
            //~ w4->change(rngMC);
            //~ w5->change(rngMC);

            if(!o.simpleSampling)
            {
                // Metropolis rejection
                double p_acc1 = std::min({1.0, exp(-(S(w1) - oldS1)/o.theta)});
                double p_acc2 = std::min({1.0, exp(-(S(w2) - oldS2)/o.theta)});
                //~ double p_acc3 = std::min({1.0, exp(-(S(w3) - oldS3)/o.theta)});
                //~ double p_acc4 = std::min({1.0, exp(-(S(w4) - oldS4)/o.theta)});
                //~ double p_acc5 = std::min({1.0, exp(-(S(w5) - oldS5)/o.theta)});
                if(p_acc1 < 1 - rngMC1())
                    w1->undoChange();
                if(p_acc2 < 1 - rngMC())
                    w2->undoChange();
                //~ if(p_acc3 < 1 - rngMC())
                    //~ w3->undoChange();
                //~ if(p_acc4 < 1 - rngMC())
                    //~ w4->undoChange();
                //~ if(p_acc5 < 1 - rngMC())
                    //~ w5->undoChange();
            }
        }

        if(Logger::verbosity >= LOG_INFO)
            oss << t_eq << " " << w1->L() << " " << w1->A()
                        << " " << w2->L() << " " << w2->A()
                        //~ << " " << w3->L() << " " << w3->A()
                        //~ << " " << w4->L() << " " << w4->A()
                        //~ << " " << w5->L() << " " << w5->A()
                        << std::endl;

        rmean1.add(S(w1));
        rmean2.add(S(w2));
        //~ rmean3.add(S(w3));
        //~ rmean4.add(S(w4));
        //~ rmean5.add(S(w5));

        // FIXME: this is a bit complicated, can it be simplified?
        // wait until the variance does not fluctuate more than "threshold" (eg. 10%) within w1
        // end when rolling mean of all other is within sqrt(variance) of w1
        double sdev = sqrt(rmean1.var());
        if(!start && t_eq >= samples && std::abs(rmean1.var()/rmean1.var(samples/2) - 1) < threshold)
            start = true;

        LOG(LOG_TOO_MUCH) << t_eq << " " << start << " "
                          << (std::abs(rmean1.mean() - rmean2.mean()) < sdev)
                          //~ << " " << (std::abs(rmean1.mean() - rmean3.mean()) < sdev)
                          //~ << " " << (std::abs(rmean1.mean() - rmean4.mean()) < sdev)
                          //~ << " " << (std::abs(rmean1.mean() - rmean5.mean()) < sdev)
                          ;

        if(start
          && std::abs(rmean1.mean() - rmean2.mean()) < sdev
          //~ && std::abs(rmean1.mean() - rmean3.mean()) < sdev
          //~ && std::abs(rmean1.mean() - rmean4.mean()) < sdev
          //~ && std::abs(rmean1.mean() - rmean5.mean()) < sdev
          )
            break;

        if(t_eq >= o.t_eqMax)
        {
            // this seems to not equilibrate, abort simulation
            return -1;
        }

        ++t_eq;
    }

    LOG(LOG_INFO) << "Equilibration estimate: t_eq = " << t_eq;
    LOG(LOG_INFO) << "plot with gnuplot (for L):";
    LOG(LOG_INFO) << "p 'equilibration.dat' u 1:2 w l t 'random', "
                     "'' u 1:4 w l t 'maxVolume', "
                     "'' u 1:6 w l t 'maxSurface', "
                     "'' u 1:8 w l t 'spiral', "
                     "'' u 1:10 w l t 'straightLine'";
    LOG(LOG_INFO) << "plot with gnuplot (for A):";
    LOG(LOG_INFO) << "p 'equilibration.dat' u 1:3 w l t 'random', "
                     "'' u 1:5 w l t 'maxVolume', "
                     "'' u 1:7 w l t 'maxSurface', "
                     "'' u 1:9 w l t 'spiral', "
                     "'' u 1:11 w l t 'straightLine'";
    return t_eq;
}

void Metropolis::run()
{
    UniformRNG rngMC(o.seedMC);

    if(!o.simpleSampling)
        oss << "# large deviation simulation at theta=" << o.theta << " and steps=" << o.steps << "\n";
    else
        oss << "# simple sampling simulation with steps=" << o.steps << "\n";
    oss << "# sweeps L A r r2 maxDiameter maxX maxY\n";

    std::unique_ptr<Walker> w;
    prepare(w);

    if(o.iterations > 0)
    {
        // do we have command line t_eq?
        if(o.t_eq == -1)
            o.t_eq = equilibrate(w, rngMC);
        // if it is still -1, the equilibrations was aborted
        if(o.t_eq == -1)
        {
            LOG(LOG_WARNING) << "Not equilibrated after " << o.t_eqMax << " sweeps. Aborting...";
            oss << "# Does not equilibrate" << std::endl;
            return;
        }

        for(int i=o.t_eq; i<o.iterations+2*o.t_eq; ++i)
        {
            // one sweep, i.e., one change try for each site
            for(int j=0; j<o.steps; ++j)
            {
                // change one random number to another random number
                double oldS = S(w);
                w->change(rngMC);
                ++trys;

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
            if(i >= 2*o.t_eq)
            {
                if(!o.conf_path.empty())
                    w->saveConfiguration(o.conf_path);

                LOG(LOG_TOO_MUCH) << "Area  : " << w->L();
                LOG(LOG_TOO_MUCH) << "Volume: " << w->A();
                LOG(LOG_DEBUG) << "Iteration: " << i;
                auto maxE = w->maxExtent();
                oss << i << " "
                    << w->L() << " "
                    << w->A() << " "
                    << w->r() << " "
                    << w->r2() << " "
                    << w->maxDiameter() << " "
                    << maxE[0] << " "
                    << maxE[1]
                    << std::endl;
            }
        }
    }

    if(!o.svg_path.empty())
        w->svg(o.svg_path, true);

    if(!o.pov_path.empty())
        w->pov(o.pov_path, true);
}
