#include "MetropolisParallelTempering.hpp"

MetropolisParallelTempering::MetropolisParallelTempering(const Cmd &o)
    : Simulation(o)
{
}

void MetropolisParallelTempering::run()
{
    const int numTemperatures = o.parallelTemperatures.size();


    // TODO: fill "mapThetaToFile"

    // create a map of the temperatures
    // since we will swap temperatures, we need to kepp track
    // which indices systems with neighboring temperatures have
    std::vector<int> thetaMap(numTemperatures);
    for(int i=0; i<numTemperatures; ++i)
        thetaMap[i] = i;

    // create all walkers, with corresponding temperatures
    // can this be done in parallel? (dimerization can take some time)
    std::vector<std::unique_ptr<Walker>> allWalkers(numTemperatures);

    #pragma omp parallel for firstprivate(o)
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
        int seedMC = ((long long)(o.seedMC+omp_get_thread_num()) * (omp_get_thread_num()+1)) % 1800000113;
        UniformRNG rngMC(seedMC);

        for(int i=0; i<o.iterations; ++i)
        {
            #pragma omp for
            for(int n=0; n<numTemperatures; ++n)
            {
                // do some sweeps (~autocorrelation time) before swapping
                // the higher this value, the lower the multithreading overhead
                // the closer to the autocorrelation time, the more efficient
                auto theta = o.parallelTemperatures[thetaMap[n]];
                for(int j=0; j<100; ++j)
                {
                    sweep(allWalkers[n], theta, rngMC);

                    // save to file (not critical, since every thread has its own file)
                    mapThetaToFile[theta] << i << " "
                                          << allWalkers[n]->L() << " "
                                          << allWalkers[n]->A() << "\n";
                }
            }

            #pragma omp barrier
            #pragma omp single
            {
                // swap neighboring temperatures
                for(int j=1; j<numTemperatures; ++j)
                {
                    auto &w_low = allWalkers[thetaMap[j-1]];
                    auto T_low = o.parallelTemperatures[thetaMap[j-1]];
                    auto &w_high = allWalkers[thetaMap[j]];
                    auto T_high = o.parallelTemperatures[thetaMap[j]];
                    auto delta = S(w_high) - S(w_low);
                    double p_acc = delta > 0. ? std::exp(-(1./T_low - 1./T_high) * delta) : 1.;
                    if(p_acc < rngMC())
                    {
                        // accepted -> update the map of the temperatures
                        auto tmp = thetaMap[j-1];
                        thetaMap[j-1] = thetaMap[j];
                        thetaMap[j] = tmp;
                    }
                }
            }
            // implicit barrier at end of single block
        }

    }
}

void MetropolisParallelTempering::sweep(std::unique_ptr<Walker> &w, double theta, UniformRNG &rngMC)
{
    // one sweep, i.e., o.sweep many change tries (default o.steps)
    for(int j=0; j<o.sweep; ++j)
    {
        // change one random number to another random number
        double oldS = S(w);
        w->change(rngMC);

        // Metropolis rejection
        double p_acc = std::exp((oldS - S(w))/theta);
        if(p_acc < rngMC())
            w->undoChange();
    }
}
