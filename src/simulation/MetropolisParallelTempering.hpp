#ifndef METROPOLISPARALLELTEMPERING_H
#define METROPOLISPARALLELTEMPERING_H

#include <iomanip>

#include "Simulation.hpp"

/** Helper Datastructure to contain swap rate statistics
 */
struct SwapStatEntry
{
    public:
        SwapStatEntry(double T1, double T2, double rate)
            : T1(T1), T2(T2), rate(rate)
        {}
        double T1;
        double T2;
        double rate;
};

/** Metropolis Monte Carlo sampling enhanced with parallel tempering.
 */
class MetropolisParallelTempering : public Simulation
{
    public:
        virtual ~MetropolisParallelTempering() {}
        MetropolisParallelTempering(const Cmd &o, const bool fileOutput=true);
        virtual void run();

        std::vector<double> proposeBetterTemperatures();

    protected:
        void sweep(std::unique_ptr<Walker> &w, double theta, UniformRNG &rngMC);

        std::vector<SwapStatEntry> swapStats;

        bool noFileOutput;
};

#endif
