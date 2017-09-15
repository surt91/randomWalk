#include "MetropolisParallelTemperingMPI.hpp"

MetropolisParallelTemperingMPI::MetropolisParallelTemperingMPI(const Cmd &o)
    : MetropolisParallelTempering(o)
{
}

void MetropolisParallelTemperingMPI::run()
{
    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Request r1, r2, r3;

    // every how many sweeps per swap trial
    const auto estimated_corr = std::max(o.steps / o.sweep, 1);

    const int numTemperatures = o.parallelTemperatures.size();
    if(numTemperatures != world_size)
    {
        LOG(LOG_ERROR) << "You need exactly as many processes as temperatures";
        LOG(LOG_ERROR) << "start with mpirun -n " << numTemperatures;
        exit(1);
    }

    std::string swapGraphName = "swapGraph" + std::to_string(o.steps) + ".dat";
    std::ofstream swapGraph(swapGraphName, std::ofstream::out);
    std::vector<int> acceptance(numTemperatures-1, 0);
    std::vector<int> swapTrial(numTemperatures-1, 0);

    // create a map of the temperatures
    // since we will swap temperatures, we need to keep track
    // where we swapped them, we need to know, which are neighbors
    std::vector<int> thetaMap(numTemperatures);
    for(int i=0; i<numTemperatures; ++i)
        thetaMap[i] = i;

    size_t max_filename_len = 0;
    for(size_t i=0; i<o.data_path_vector.size(); ++i)
        if(o.data_path_vector[i].size() > max_filename_len)
            max_filename_len = o.data_path_vector[i].size();

    if(rank == 0)
    {
        for(int i=0; i<numTemperatures; ++i)
        {
            // this will send to self, and must be non-blocking
            // send filenames to all processes
            auto tmp = o.data_path_vector[thetaMap[i]].c_str();
            MPI_Isend(tmp, max_filename_len, MPI_BYTE, i, 0, MPI_COMM_WORLD, &r1);
            // send temperatures to all processes
            auto theta = o.parallelTemperatures[thetaMap[i]];
            MPI_Isend(&theta, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &r2);
        }
    }

    char *filename = new char[max_filename_len];
    MPI_Recv(filename, max_filename_len, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    std::ofstream current_stream(filename, std::ofstream::out);

    double theta;
    MPI_Recv(&theta, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if(rank == 0)
    {
        MPI_Wait(&r1, MPI_STATUS_IGNORE);
        MPI_Wait(&r2, MPI_STATUS_IGNORE);
    }

    // give every Thread a different seed
    // ensure that they do not overflow
    // FIXME: think about a better seed
    const int seedMC = ((long long)(o.seedMC+omp_get_thread_num()) * (omp_get_thread_num()+1)) % 1800000113;
    UniformRNG rngMC(seedMC);
    std::unique_ptr<Walker> walker;

    for(int n=0; n<numTemperatures; ++n)
    {
        Cmd tmp(o);
        tmp.seedRealization = ((long long)(tmp.seedRealization + n) * (n+1)) % 1800000121;
        prepare(walker, tmp);
    }

    for(int i=0; i<o.iterations+2*o.t_eq; )
    {
        // do some sweeps (~fasted autocorrelation time) before swapping
        // the higher this value, the lower the multithreading overhead
        // the lower, the more swaps can be performed
        for(int j=0; j<estimated_corr; ++j)
        {
            sweep(walker, theta, rngMC);

            // save to file (not critical, since every process has its own file)
            if(i >= 2*o.t_eq)
            {
                current_stream << i+j << " "
                                    << walker->L() << " "
                                    << walker->A() << " ";

                // simple sampling: signaled by the Planck temperature
                // inf or nan do not work with gcc's -ffast-math
                if(theta >= 1.4e32)
                {
                    auto maxE = walker->maxExtent();
                    current_stream
                        << walker->r() << " "
                        << walker->r2() << " "
                        << walker->maxDiameter() << " "
                        << maxE[0] << " "
                        << maxE[1] << " "
                        << walker->rx() << " "
                        << walker->ry();
                }
                current_stream << std::endl; //yes, I want to flush explicitly
            }
        }
        i += estimated_corr;

        // critical region:
        //  * blocking receive to wait for all processes
        //  * save data
        //  * swap temperatures
        //  * send new temperatures and filenames to all processes

        // non-blocking send to send observables
        double observable = S(walker);
        std::vector<double> observables(numTemperatures); // buffer to collect observables (only rank == 0)
        MPI_Gather(&observable, 1, MPI_DOUBLE, &observables[0], 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        if(rank == 0)
        {
            // attempt n-1 swaps of neighboring temperatures
            for(int k=0; k<numTemperatures-1; ++k)
            {
                // determine which pair to swap, can appear multiple times
                const int j = rngMC() * (numTemperatures-1) + 1;

                double &s_1 = observables[j-1];
                const double T_1 = o.parallelTemperatures[thetaMap[j-1]];
                double &s_2 = observables[j];
                const double T_2 = o.parallelTemperatures[thetaMap[j]];
                const double p_acc = std::exp((1./T_2 - 1./T_1) * (s_2 - s_1));

                if(p_acc > rngMC())
                {
                    LOG(LOG_TOO_MUCH) << "(" << i << ") swap: " << thetaMap[j-1] << " = " <<  T_1 << " <-> " <<  thetaMap[j] << " = " <<  T_2;

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

            // scatter filenames
            for(int i=0; i<numTemperatures; ++i)
            {
                auto tmp = o.data_path_vector[thetaMap[i]].c_str();
                MPI_Isend(tmp, max_filename_len, MPI_BYTE, i, 0, MPI_COMM_WORLD, &r3);
            }
        }
        // scatter temperatures (scatter will also read them in the receiving ranks)
        MPI_Scatter(&thetaMap[0], 1, MPI_DOUBLE, &theta, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        if(rank == 0)
            MPI_Wait(&r3, MPI_STATUS_IGNORE);
    }

    // critical region for statistics
    if(rank == 0)
    {
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
            std::ofstream oss(o.data_path_vector[i], std::ofstream::out);
            oss << ss.str();
            footer(oss);

            gzip(o.data_path_vector[i]);
        }
    }

    MPI_Finalize();

    delete[] filename;
}
