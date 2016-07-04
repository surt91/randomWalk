#pragma once

#include <string>
#include <sstream>

#include <tclap/CmdLine.h>

#include "Logging.hpp"

// test, if we are using openmp
#ifdef _OPENMP
   #include <omp.h>
#else
   #define omp_get_thread_num() 0
   #define omp_set_num_threads(x)
#endif

enum hull_algorithm_t {
    CH_QHULL = 1,   //< use the quick hull implementation Qhull
    CH_QHULL_AKL,   //< use the quick hull implementation Qhull with Akl's heuristic
    CH_ANDREWS,     //< use Andrews monotone chain algorithm
    CH_ANDREWS_AKL, //< use Andrews monotone chain algorithm with Akl's heuristic
    CH_GRAHAM,      //< [not implemented]
    CH_GRAHAM_AKL,  //< [not implemented]
    CH_JARVIS,      //< [not implemented]
    CH_JARVIS_AKL,  //< [not implemented]
    CH_CHAN,        //< [not implemented]
    CH_CHAN_AKL     //< [not implemented]
};

const std::vector<std::string> CH_LABEL = {
    "nan",
    "QHull",
    "QHull + Akl",
    "Andrews",
    "Andrews + Akl",
    "Graham",
    "Graham + Akl",
    "Jarvis",
    "Jarvis + Akl",
    "Chan",
    "Chan + Akl"
};

enum walk_type_t {
    WT_RANDOM_WALK = 1,             //< Lattice random walk on hypercubic lattice with steplength = 1
    WT_LOOP_ERASED_RANDOM_WALK,     //< Loop Erased random walk on hypercubic lattice
    WT_SELF_AVOIDING_RANDOM_WALK,   //< Self-Avoiding random walk on hypercubic lattice
    WT_REAL_RANDOM_WALK,            //< Random direction, steplength = 1
    WT_GAUSSIAN_RANDOM_WALK,        //< Gaussian random walk
    WT_LEVY_FLIGHT,                 //< Levy flight
    WT_CORRELATED_RANDOM_WALK,      //< Correlated random walk
};

const std::vector<std::string> TYPE_LABEL = {
    "nan",
    "Random Walk",
    "Loop Erased Random Walk",
    "Self-Avoiding Random Walk",
    "Real Random Walk",
    "Gaussian Random Walk",
    "Levy Flight",
    "Correlated Random Walk",
};

enum wanted_observable_t {
    WO_SURFACE_AREA = 1,    ///< eg. circumference in d=2
    WO_VOLUME               ///< eg. area in d=2
};

const std::vector<std::string> WANTED_OBSERVABLE_LABEL = {
    "nan",
    "surface area (circumference in d=2)",
    "volume (area in d=2)"
};

enum sampling_method_t {
    SM_METROPOLIS = 1,    ///< Metropolis sampling with a artificial temp
    SM_WANG_LANDAU,       ///< Direct WangLandau sampling of the distribution
    SM_FAST_WANG_LANDAU,  ///< Direct 1/t FastWangLandau sampling of the distribution
    SM_METROPOLIS_PARALLEL_TEMPERING  ///< Metropolis sampling enhanced with parallel tempering
};

const std::vector<std::string> SAMPLING_METHOD_LABEL = {
    "nan",
    "Metropolis",
    "Wang Landau",
    "Fast 1/t Wang Landau",
    "Metropolis and Parallel Tempering",
};

/** Command line parser.
 *
 * This command line parser uses TCLAP (http://tclap.sourceforge.net/).
 */
class Cmd
{
    public:
        Cmd()
            : mu(0.0),
              sigma(1.0)
            {}
        Cmd(int argc, char** argv);

        std::string tmp_path;
        std::string data_path;
        std::string conf_path;
        std::vector<std::string> data_path_vector; ///< vector of output names, one for every temperature (only for parallel tempering);
        std::vector<std::string> conf_path_vector; ///< vector of output names, one for every temperature (only for parallel tempering);
        std::string svg_path;
        std::string pov_path;
        std::string gp_path;
        int steps;
        int numWalker;
        int seedRealization;
        int seedMC;
        walk_type_t type;
        int d;
        int iterations;
        int sweep;
        int t_eq;
        int t_eqMax;
        int parallel;
        double theta;
        std::vector<double> parallelTemperatures;
        bool simpleSampling;
        std::vector<double> wangLandauBorders;
        int wangLandauBins;
        int wangLandauOverlap;
        sampling_method_t sampling_method;
        hull_algorithm_t chAlg;
        wanted_observable_t wantedObservable;

        double mu;
        double sigma;
        double lnf_min;
        double flatness_criterion;

        bool onlyBounds;
        bool onlyCenters;

        bool benchmark;
        double benchmark_A;
        double benchmark_L;

        std::string text;
};
