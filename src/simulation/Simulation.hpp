#pragma once

#include <cmath>
#include <functional>
#include <climits>

// test, if we are using openmp
#ifdef _OPENMP
   #include <omp.h>
#else
   #define omp_get_thread_num() 0
   #define omp_set_num_threads(x) 1
#endif

#include "../Cmd.hpp"
#include "../Logging.hpp"
#include "../stat.hpp"
#include "../walker/Walker.hpp"
#include "../walker/LatticeWalker.hpp"
#include "../walker/LoopErasedWalker.hpp"
#include "../walker/SelfAvoidingWalker.hpp"
#include "../walker/RealWalker.hpp"
#include "../walker/GaussWalker.hpp"
#include "../walker/LevyWalker.hpp"
#include "../RNG.hpp"

/** Abstract Base Class, derive classes that sample random walks.
 */
class Simulation
{
    public:
        Simulation(const Cmd &o);
        virtual ~Simulation();

        virtual void run() = 0;

        void mute() {muted=true;};

        double sum_L;
        double sum_A;
        double sum_r;
        double sum_r2;

    protected:
        Cmd o;
        int fails;
        int tries;
        void prepare(std::unique_ptr<Walker>& w);
        std::function<double(std::unique_ptr<Walker>&)> S;
        std::ofstream oss;
        bool muted;

    private:
        clock_t begin;
};
