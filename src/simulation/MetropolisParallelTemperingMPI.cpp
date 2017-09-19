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
    if(rank == 0)
        for(int i=0; i<numTemperatures; ++i)
            thetaMap[i] = i;

    // initialize output files
    if(rank == 0)
        for(int i=0; i<numTemperatures; ++i)
        {
            std::ofstream oss(o.data_path_vector[i], std::ofstream::out);
            header(oss);
        }

    // find length of longest outputfilename
    // we will need it for efficient scattering to the processes
    size_t max_filename_len = 0;
    for(size_t i=0; i<o.data_path_vector.size(); ++i)
        if(o.data_path_vector[i].size() > max_filename_len)
            max_filename_len = o.data_path_vector[i].size() + 1; // +1 for \0

    // allocate resources to scatter...
    std::vector<char> filename_array(max_filename_len * numTemperatures, '\0');
    std::vector<double> sorted_temperatures(numTemperatures, 0);

    // ... and where they are scattered to
    double theta;
    char *filename = new char[max_filename_len];

    // give every Thread a different seed
    // ensure that they do not overflow
    // rank is deterministic
    const int seedMC = ((uint64_t)(o.seedMC+rank) * (rank+1)) % 1800000113;
    UniformRNG rngMC(seedMC);
    std::unique_ptr<Walker> walker;

    o.seedRealization = ((uint64_t)(o.seedRealization + rank) * (rank+1)) % 1800000121;
    prepare(walker, o);

    for(int i=0; i<o.iterations+2*o.t_eq; )
    {
        // prepare the information for the next sweep:
        // which temperature will be assigned to which process
        if(rank == 0)
            for(int i=0; i<numTemperatures; ++i)
            {
                auto tmp = o.data_path_vector[thetaMap[i]].c_str();
                std::strcpy(&filename_array[i*max_filename_len], tmp);
                auto theta = o.parallelTemperatures[thetaMap[i]];
                sorted_temperatures[i] = theta;
            }

        // scatter the specifications to the processes
        MPI_Scatter(&sorted_temperatures[0], 1, MPI_DOUBLE, &theta, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Scatter(&filename_array[0], max_filename_len, MPI_BYTE, filename, max_filename_len, MPI_CHAR, 0, MPI_COMM_WORLD);

        std::ofstream current_stream(filename, std::ofstream::app);

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

        // gather will synchronize, pass observables to master
        double observable = S(walker);
        std::vector<double> observables(numTemperatures); // buffer to collect observables (only rank == 0)
        MPI_Gather(&observable, 1, MPI_DOUBLE, &observables[0], 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // master decides which temperatures to swap
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

                checksum += observables[k];
            }

            // detailed data about the swaps
            swapGraph << i << " ";
            for(int j=1; j<numTemperatures; ++j)
                swapGraph << thetaMap[j] << " ";
            swapGraph << "\n";
        }
    }

    // critical region for statistics
    if(rank == 0)
    {
        LOG(LOG_INFO) << "# acceptance: " << acceptance;
        LOG(LOG_INFO) << "# trials    : " << swapTrial;
        std::stringstream ss;
        ss << "# swap success rates:\n";
        for(int j=0; j<numTemperatures-1; ++j)
            ss << "#    " << (int)((double)acceptance[j]/swapTrial[j]*100.0) << "% (" << acceptance[j] << "/" << swapTrial[j] << ")" << " : " << o.parallelTemperatures[j] << " <-> " << o.parallelTemperatures[j+1] << "\n";
        LOG(LOG_INFO) << ss.str();

        swapGraph.close();
        gzip(swapGraphName);

        for(int i=0; i<numTemperatures; ++i)
        {
            std::ofstream oss(o.data_path_vector[i], std::ofstream::app);
            oss << ss.str();
            footer(oss);

            gzip(o.data_path_vector[i]);
        }
    }

    MPI_Finalize();

    delete[] filename;
}
