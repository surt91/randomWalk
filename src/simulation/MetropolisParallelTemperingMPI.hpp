#pragma once

#include <mpi.h>

#include "MetropolisParallelTempering.hpp"

/** Metropolis Monte Carlo sampling enhanced with parallel tempering using MPI.
 */
class MetropolisParallelTemperingMPI : public MetropolisParallelTempering
{
    public:
        MetropolisParallelTemperingMPI(const Cmd &o);
        virtual void run();
};
