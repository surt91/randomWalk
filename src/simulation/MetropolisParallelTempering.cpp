#include "MetropolisParallelTempering.hpp"

MetropolisParallelTempering::MetropolisParallelTempering(const Cmd &o)
    : Simulation(o, false)
{
}

void MetropolisParallelTempering::run()
{
    // every how many sweeps per swap trial
    const auto estimated_corr = std::max(o.steps / o.sweep, 1);

    const int numTemperatures = o.parallelTemperatures.size();

    std::string swapGraphName = "swapGraph" + std::to_string(o.steps) + ".dat";
    std::ofstream swapGraph(swapGraphName, std::ofstream::out);

    // create a map of the temperatures
    // since we will swap temperatures, we need to keep track
    // where we swapped them, we need to know, which are neighbors
    std::vector<int> thetaMap(numTemperatures);
    for(int i=0; i<numTemperatures; ++i)
        thetaMap[i] = i;

    std::vector<std::unique_ptr<std::ofstream>> files;
    for(int i=0; i<numTemperatures; ++i)
    {
        // this looks like a leak, but unique pointer saves the day
        files.emplace_back(new std::ofstream(o.data_path_vector[i], std::ofstream::out));
        header(*files[i]);
    }

    // create all walkers, with corresponding temperatures
    // can this be done in parallel? (dimerization can take some time)
    std::vector<std::unique_ptr<Walker>> allWalkers(numTemperatures);
    std::vector<int> acceptance(numTemperatures-1, 0);
    std::vector<int> swapTrial(numTemperatures-1, 0);

    // for loop schedule:
    // static schedule for NUMA locality and chunk size 1, to mix the
    // temperatures inside the threads, i.e., we do not want to have a
    // thread calculating all lowest temperatures -> all threads should
    // need roughly equal time

    #pragma omp parallel
    {
        // give every Thread a different seed
        // ensure that they do not overflow
        // FIXME: think about a better seed
        const int seedMC = ((long long)(o.seedMC+omp_get_thread_num()) * (omp_get_thread_num()+1)) % 1800000113;
        UniformRNG rngMC(seedMC);

        // init the walks in parallel -> crucial for NUMA memory locality
        #pragma omp for schedule(static, 1)
        for(int n=0; n<numTemperatures; ++n)
        {
            Cmd tmp(o);
            tmp.seedRealization = ((long long)(tmp.seedRealization + n) * (n+1)) % 1800000121;
            prepare(allWalkers[n], tmp);
        }

        for(int i=0; i<o.iterations+2*o.t_eq; )
        {
            #pragma omp for schedule(static, 1)
            for(int n=0; n<numTemperatures; ++n)
            {
                // do some sweeps (~fasted autocorrelation time) before swapping
                // the higher this value, the lower the multithreading overhead
                // the lower, the more swaps can be performed
                const auto theta = o.parallelTemperatures[thetaMap[n]];
                for(int j=0; j<estimated_corr; ++j)
                {
                    sweep(allWalkers[n], theta, rngMC);

                    // save to file (not critical, since every thread has its own file)
                    if(i >= 2*o.t_eq)
                    {
                        *files[thetaMap[n]] << i+j << " "
                                            << allWalkers[n]->L() << " "
                                            << allWalkers[n]->A() << " ";

                        // simple sampling: signaled by the Planck temperature
                        // inf or nan do not work with gcc's -ffast-math
                        if(theta >= 1.4e32)
                        {
                            auto maxE = allWalkers[n]->maxExtent();
                            *files[thetaMap[n]]
                                << allWalkers[n]->r() << " "
                                << allWalkers[n]->r2() << " "
                                << allWalkers[n]->maxDiameter() << " "
                                << maxE[0] << " "
                                << maxE[1] << " "
                                << allWalkers[n]->rx() << " "
                                << allWalkers[n]->ry();
                        }
                        *files[thetaMap[n]] << std::endl; //yes, I want to explicitly flush
                    }
                }
            }
            i += estimated_corr;

            #pragma omp barrier
            #pragma omp master
            // master instead of single, for determinism
            {
                // attempt n-1 swaps of neighboring temperatures
                for(int k=0; k<numTemperatures-1; ++k)
                {
                    // determine which pair to swap, can appear multiple times
                    const int j = rngMC() * (numTemperatures-1) + 1;

                    auto &w_low = allWalkers[j-1];
                    const auto T_low = o.parallelTemperatures[thetaMap[j-1]];
                    auto &w_high = allWalkers[j];
                    const auto T_high = o.parallelTemperatures[thetaMap[j]];
                    const auto delta = S(w_high) - S(w_low);
                    const double p_acc = delta > 0. ? std::exp(-(1./T_low - 1./T_high) * delta) : 1.;

                    if(p_acc > rngMC())
                    {
                        LOG(LOG_TOO_MUCH) << "(" << i << ") swap: " << thetaMap[j-1] << " = " <<  T_low << " <-> " <<  thetaMap[j] << " = " <<  T_high;

                        // accepted -> update the map of the temperatures
                        std::swap(thetaMap[j-1], thetaMap[j]);

                        acceptance[j-1] += 1;
                    }
                    swapTrial[j-1] += 1;
                }

                // detailed data about the swaps
                swapGraph << i << " ";
                for(int j=1; j<numTemperatures; ++j)
                    swapGraph << thetaMap[j] << " ";
                swapGraph << "\n";
            }
            #pragma omp barrier
        }
    }

    LOG(LOG_INFO) << "# acceptance: " << acceptance;
    LOG(LOG_INFO) << "# trials    : " << swapTrial;
    std::stringstream ss;
    ss << "# swap success rates:\n";
    for(int j=0; j<numTemperatures-1; ++j)
        ss << "#    " << (int)((double)acceptance[j]/swapTrial[j]*100.0) << "%" << " : " << o.parallelTemperatures[j] << " <-> " << o.parallelTemperatures[j+1] << "\n";
    LOG(LOG_INFO) << ss.str();

    swapGraph.close();
    gzip(swapGraphName);

    for(int i=0; i<numTemperatures; ++i)
    {
        *files[i] << ss.str();
        footer(*files[i]);

        gzip(o.data_path_vector[i]);
    }
}

void MetropolisParallelTempering::sweep(std::unique_ptr<Walker> &w, double theta, UniformRNG &rngMC)
{
    // one sweep, i.e., o.sweep many change tries (default o.steps)
    for(int j=0; j<o.sweep; ++j)
    {
        // change one random number to another random number
        const double oldS = S(w);
        w->change(rngMC);

        // Metropolis rejection
        const double p_acc = std::exp((oldS - S(w))/theta);
        if(p_acc < rngMC())
            w->undoChange();
    }
}
