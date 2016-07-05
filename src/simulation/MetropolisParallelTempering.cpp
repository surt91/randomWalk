#include "MetropolisParallelTempering.hpp"

MetropolisParallelTempering::MetropolisParallelTempering(const Cmd &o)
    : Simulation(o)
{
}

void MetropolisParallelTempering::run()
{
    // every how many sweeps per swap trial
    const auto estimated_corr = 100;

    const int numTemperatures = o.parallelTemperatures.size();

    std::string swapGraphName = "swapGraph.dat";
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
    std::vector<int> acceptance(numTemperatures, 0);
    std::vector<int> swapTrial(numTemperatures, 0);

    for(int n=0; n<numTemperatures; ++n)
    {
        o.seedRealization = ((long long)(o.seedRealization + n) * (n+1)) % 1800000121;
        prepare(allWalkers[n], o);
    }

    #pragma omp parallel
    {
        // give every Thread a different seed
        // ensure that they do not overflow
        // FIXME: think about a better seed
        const int seedMC = ((long long)(o.seedMC+omp_get_thread_num()) * (omp_get_thread_num()+1)) % 1800000113;
        UniformRNG rngMC(seedMC);

        for(int i=0; i<o.iterations; )
        {
            #pragma omp for
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
                    *files[thetaMap[n]] << i+j << " "
                                        << allWalkers[n]->L() << " "
                                        << allWalkers[n]->A() << "\n";
                }
            }
            i += estimated_corr;

            #pragma omp barrier
            #pragma omp master
            // master instead of single, for determinism
            {
                // swap neighboring temperatures
                for(int j=1; j<numTemperatures; ++j)
                {
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
                        auto tmp = thetaMap[j-1];
                        thetaMap[j-1] = thetaMap[j];
                        thetaMap[j] = tmp;

                        acceptance[j] += 1;
                    }

                    swapTrial[j] += 1;
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
    for(int j=1; j<numTemperatures; ++j)
        ss << "#    " << (int)((double)acceptance[j]/swapTrial[j]*100.0) << "%" << " : " << o.parallelTemperatures[j-1] << " <-> " << o.parallelTemperatures[j] << "\n";
    LOG(LOG_INFO) << ss.str();

    std::string cmd("gzip -f ");
    swapGraph.close();
    system((cmd+swapGraphName).c_str());

    for(int i=0; i<numTemperatures; ++i)
    {
        *files[i] << ss.str();
        footer(*files[i]);

        system((cmd+o.data_path_vector[i]).c_str());
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
