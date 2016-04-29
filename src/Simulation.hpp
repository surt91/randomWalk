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

#include "Cmd.hpp"
#include "Logging.hpp"
#include "Walker.hpp"
#include "LatticeWalker.hpp"
#include "LoopErasedWalker.hpp"
#include "SelfAvoidingWalker.hpp"
#include "RealWalker.hpp"
#include "GaussWalker.hpp"
#include "RNG.hpp"
#include "Benchmark.hpp"

class Simulation
{
    public:
        Simulation(const Cmd &o);
        virtual ~Simulation();

        virtual void run() = 0;

    protected:
        Cmd o;
        int fails;
        int tries;
        void prepare(std::unique_ptr<Walker>& w);
        std::function<double(std::unique_ptr<Walker>&)> S;
        std::ofstream oss;

    private:
        clock_t begin;
};
