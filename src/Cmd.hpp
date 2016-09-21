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
    CH_NOP = 0,     //< do not calculate the hull, just return 0
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
    "NOP, do nothing",
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

        std::string tmp_path;                       ///< path to store temporary files
        std::string data_path;                      ///< full path of output file
        std::string conf_path;                      ///< full path to store the full configuration of the walk
        std::vector<std::string> data_path_vector;  ///< vector of output names, one for every temperature (only for parallel tempering);
        std::vector<std::string> conf_path_vector;  ///< vector of output names, one for every temperature (only for parallel tempering);
        std::string svg_path;                       ///< path to store a SVG image of one \f$d=2\f$ walk
        std::string pov_path;                       ///< path to store a povray file of one \f$d=3\f$ walk
        std::string gp_path;                        ///< path to store a gnuplot file of one \f$d=2,3\f$ walk
        int steps;                                  ///< how many steps \f$T\f$ th walk takes
        int numWalker;                              ///< how many independet walkers should be simulated (hull of their union)
        int seedRealization;                        ///< RNG seed to construct the initial walk realization
        int seedMC;                                 ///< RNG seed for randomness needed in the Monte Carlo simulation
        walk_type_t type;                           ///< type of walk to be simulated
        int d;                                      ///< space dimension in which the walker lives
        int iterations;                             ///< number of iterations (e.g. sweeps) in the Monte Carlo simulation
        int sweep;                                  ///< how many trial changes constitute one sweep
        int t_eq;                                   ///< equilibration time
        int t_eqMax;                                ///< time after which to abort equilibration attempts
        int parallel;                               ///< number of processors used in parallel
        double theta;                               ///< temperature \f$\Theta\f$ to simulate at (only Metropolis type simulations)
        std::vector<double> parallelTemperatures;   ///< temperatures \f$\Theta\f$ to simulate at (only parallel tempering type simulations)
        bool simpleSampling;                        ///< use naive simple sampling
        std::vector<double> wangLandauBorders;      ///< borders of the Wang Landau bins (only Wang Landau type simulations)
        int wangLandauBins;                         ///< number of Wang Landau bins
        int wangLandauOverlap;                      ///< overlap between Wang Landau ranges in bins
        sampling_method_t sampling_method;          ///< sampling method to use (Metropolis or Wang Landau type)
        hull_algorithm_t chAlg;                     ///< convex hull algorithm to use
        wanted_observable_t wantedObservable;       ///< which observable to study

        double mu;                  ///< mean \f$\mu\f$ of the Gaussian to draw random numbers from (only correlated walks)
        double sigma;               ///< standard deviation \f$\sigma\f$ of the Gaussian to draw random numbers from (only correlated walks)
        double lnf_min;             ///< minimum refinement factor up to which is simulated (only Wang Landau type simulations)
        double flatness_criterion;  ///< flatness criterion to be used (only Wang Landau type simulations)

        bool onlyBounds;            ///< output only the maximum and minimum of the observable and exit
        bool onlyCenters;           ///< output only the centers of the Wang Landau bins and exit
        bool onlyLERWExample;       ///< save a LERW with visualized erased loops and exit
        bool onlyPivotExample;      ///< save a picture of a pivot step and exit

        bool benchmark;             ///< perform benchmarks and exit
        double benchmark_A;         ///< helper to store expected value of the benchmark
        double benchmark_L;         ///< helper to store expected value of the benchmark

        std::string text;           ///< the full command used to start this program
};
