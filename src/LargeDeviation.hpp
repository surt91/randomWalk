#pragma once

#include <cmath>
#include <functional>
#include <climits>

// test, if we are using openmp
#ifdef _OPENMP
   #include <omp.h>
#else
   #define omp_get_thread_num() 0
#endif

#include "Cmd.hpp"
#include "Logging.hpp"
#include "Walker.hpp"
#include "LoopErasedWalker.hpp"
#include "SelfAvoidingWalker.hpp"
#include "RNG.hpp"
#include "Benchmark.hpp"
#include "stat.hpp"
#include "Histogram.hpp"
#include "DensityOfStates.hpp"

void metropolis(const Cmd &o);
void wang_landau(const Cmd &o);
