#pragma once

#include <cmath>
#include <ctime>
#include <functional>
#include <climits>
#include <cstdint>

// test, if we are using openmp
#ifdef _OPENMP
   #include <omp.h>
#else
   #define omp_get_thread_num() 0
   #define omp_set_num_threads(x)
#endif

#include "../Cmd.hpp"
#include "../Logging.hpp"
#include "../stat/stat.hpp"
#include "../stat/runtime.hpp"
#include "../walker/Walker.hpp"
#include "../walker/MultipleWalker.hpp"
#include "../walker/LatticeWalker.hpp"
#include "../walker/LoopErasedWalker.hpp"
#include "../walker/SelfAvoidingWalker.hpp"
#include "../walker/RealWalker.hpp"
#include "../walker/GaussWalker.hpp"
#include "../walker/LevyWalker.hpp"
#include "../walker/CorrelatedWalker.hpp"
#include "../walker/EscapeWalker.hpp"
#include "../walker/ScentWalker.hpp"
#include "../RNG.hpp"
#include "../io.hpp"

/** Abstract Base Class, derive classes that sample random walks.
 */
class Simulation
{
    public:
        Simulation(const Cmd &o, const bool fileOutput=true);
        virtual ~Simulation();

        virtual void run() = 0;
        static void prepare(std::unique_ptr<Walker>& w, const Cmd &o);
        static std::function<double(const std::unique_ptr<Walker>&)> prepareS(const Cmd &o);
        static double getLowerBound(Cmd &o);
        static double getUpperBound(Cmd &o);
        static double getReasonalbleUpperBound(Cmd &o);
        static double getReasonalbleLowerBound(Cmd &o);

        void mute() {muted=true;}

        double sum_L;
        double sum_A;
        double sum_r;
        double sum_r2;

        virtual double check() {return checksum;} ///< A value which can be checked against a known value in tests to find regressions

    protected:
        Cmd o;
        uint64_t fails;
        uint64_t tries;
        std::function<double(std::unique_ptr<Walker>&)> S;
        std::ofstream oss;
        bool muted;
        bool fileOutput;

        double checksum;

        void header(std::ofstream &oss);
        void footer(std::ofstream &oss);

    private:
        clock_t start;
};
