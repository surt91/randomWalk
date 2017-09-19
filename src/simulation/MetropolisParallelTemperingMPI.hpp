#ifndef METROPOLISPARALLELTEMPERINGMPI_H
#define METROPOLISPARALLELTEMPERINGMPI_H

#include <mpi.h>
#include <cstring>

#include "MetropolisParallelTempering.hpp"

/** Metropolis Monte Carlo sampling enhanced with parallel tempering using MPI.
 */
class MetropolisParallelTemperingMPI : public MetropolisParallelTempering
{
    public:
        MetropolisParallelTemperingMPI(const Cmd &o);
        virtual void run();
};

#endif
