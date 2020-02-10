#ifndef CMD_H
#define CMD_H

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
    CH_NOP = 0,     ///< do not calculate the hull, just return 0
    CH_QHULL = 1,   ///< use the quick hull implementation Qhull
    CH_QHULL_AKL,   ///< use the quick hull implementation Qhull with Akl's heuristic
    CH_ANDREWS,     ///< use Andrews monotone chain algorithm
    CH_ANDREWS_AKL, ///< use Andrews monotone chain algorithm with Akl's heuristic
    CH_GRAHAM,      ///< [not implemented]
    CH_GRAHAM_AKL,  ///< [not implemented]
    CH_JARVIS,      ///< use Jarvis' march (gift wrapping)
    CH_JARVIS_AKL,  ///< use Jarvis' march (gift wrapping) with Akl's heuristic
    CH_CHAN,        ///< [not implemented]
    CH_CHAN_AKL,    ///< [not implemented]
    CH_1D           ///< boring case of 1 dimensional hull
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
    "Chan + Akl",
    "one dimensional"
};

enum walk_type_t {
    WT_RANDOM_WALK = 1,             ///< Lattice random walk on hypercubic lattice with steplength = 1
    WT_LOOP_ERASED_RANDOM_WALK,     ///< Loop Erased random walk on hypercubic lattice
    WT_SELF_AVOIDING_RANDOM_WALK,   ///< Self-Avoiding random walk on hypercubic lattice
    WT_REAL_RANDOM_WALK,            ///< Random direction, steplength = 1
    WT_GAUSSIAN_RANDOM_WALK,        ///< Gaussian random walk
    WT_LEVY_FLIGHT,                 ///< Levy flight
    WT_CORRELATED_RANDOM_WALK,      ///< Correlated random walk
    WT_ESCAPE_RANDOM_WALK,          ///< Escape random walk
    WT_SCENT_RANDOM_WALK,           ///< Scent random walk
    WT_TRUE_SELF_AVOIDING_WALK,     ///< "True" Self-avoiding walk
    WT_RESET_WALK,                  ///< resetting random walk
    WT_BRANCH_WALK,                 ///< branching Gaussian walk
    WT_RUNANDTUMBLE_WALK,           ///< run-and-tumble walk (fixed n)
    WT_RUNANDTUMBLE_T_WALK,         ///< run-and-tumble walk (fixed t)
    WT_RETURNING_LATTICE_WALK,      ///< Lattice random walk returning to origin
    WT_GAUSSIAN_RESET_WALK,         ///< resetting gaussian random walk
    WT_BROWNIAN_RESET_WALK,         ///< resetting Brownian motion
    WT_BROWNIAN_RESET_WALK_SHIFTED, ///< resetting Brownian motion with shift
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
    "Escape Random Walk",
    "Scent Random Walk",
    "'True' Self-Avoiding Walk",
    "Resetting Random Walk",
    "Branching Gaussian Walk",
    "Run-and-tumble Walk (fixed n)",
    "Run-and-tumble Walk (fixed t)",
    "Returning Lattice Random Walk",
    "Gaussian Resetting Random Walk",
    "Resetting Brownian motion",
    "Resetting Brownian motion with shift",
};

enum wanted_observable_t {
    WO_SURFACE_AREA = 1,    ///< eg. circumference in d=2
    WO_VOLUME,              ///< eg. area in d=2
    WO_PASSAGE              ///< time needed to change the sign of the x component
};

const std::vector<std::string> WANTED_OBSERVABLE_LABEL = {
    "nan",
    "surface area (circumference in d=2)",
    "volume (area in d=2)",
    "passage time"
};

enum sampling_method_t {
    SM_SIMPLESAMPLING = 0,///< Simple Sampling, create new walks from scratch
    SM_METROPOLIS = 1,    ///< Metropolis sampling with a artificial temp
    SM_WANG_LANDAU,       ///< Direct WangLandau sampling of the distribution
    SM_FAST_WANG_LANDAU,  ///< Direct 1/t FastWLEntropic sampling of the distribution
    SM_METROPOLIS_PARALLEL_TEMPERING,     ///< Metropolis sampling enhanced with parallel tempering
    SM_METROPOLIS_PARALLEL_TEMPERING_MPI  ///< Metropolis sampling enhanced with parallel tempering using MPI
};

const std::vector<std::string> SAMPLING_METHOD_LABEL = {
    "Simple Sampling",
    "Metropolis",
    "Wang Landau",
    "Fast 1/t Wang Landau",
    "Metropolis and Parallel Tempering",
    "Metropolis and Parallel Tempering MPI",
};

enum agent_start_t {
    AS_RANDOM = 0,      ///< Start the agents randomly in the plane
    AS_CIRCLE = 1,      ///< Start the agents in a circle around the chosen one
    AS_TRIANGULAR = 2,  ///< Start the agents on the sites of an triangular lattice
    AS_RELAXED = 3,     ///< Start the agents with an already relaxed configuration
};

const std::vector<std::string> AGENT_START_LABEL = {
    "random",
    "circular",
    "triangular",
    "relaxed",
};

/** Command line parser.
 *
 * This command line parser uses TCLAP (http://tclap.sourceforge.net/).
 */
class Cmd
{
    public:
        // these are sane default values
        Cmd()
            : tmp_path("."),
              data_path("out.dat"),
              conf_path(),
              data_path_vector(),
              conf_path_vector(),
              svg_path(),
              pov_path(),
              gp_path(),
              threejs_path(),
              steps(100),
              numWalker(1),
              seedRealization(0),
              seedMC(0),
              type(WT_RANDOM_WALK),
              d(2),
              iterations(100),
              sweep(-1),
              t_eq(-1),
              t_eqMax(1e5),
              parallel(1),
              theta(1e4),
              parallelTemperatures(),
              simpleSampling(false),
              wangLandauBorders(),
              wangLandauBins(100),
              wangLandauOverlap(10),
              sampling_method(SM_SIMPLESAMPLING),
              chAlg(CH_QHULL),
              wantedObservable(WO_VOLUME),
              passageTimeStart(0),
              passageTimeStarts(),
              mu(0.0),
              sigma(1.0),
              beta(0.0),
              resetrate(0.0),
              shift(0.0),
              gamma(1.0),
              total_length(10.5),
              width(10),
              tas(1000),
              agent_start(AS_RANDOM),
              lnf_min(1e-8),
              flatness_criterion(0.8),
              onlyBounds(false),
              onlyCenters(false),
              onlyLERWExample(false),
              onlyPivotExample(false),
              onlyPTTemperatures(false),
              text()
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
        std::string threejs_path;                   ///< path to store a three.js html file of one \f$d=2,3\f$ walk
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
        int passageTimeStart;                       ///< first reference point after which to look for sign changes
        std::vector<int> passageTimeStarts;         ///< reference points after which to look for sign changes

        double mu;                  ///< mean \f$\mu\f$ of the Gaussian to draw random numbers from (only correlated walks)
        double sigma;               ///< standard deviation \f$\sigma\f$ of the Gaussian to draw random numbers from (only correlated walks)

        double beta;                ///< avoidance parameter, step on visited sites with exp(-beta N) (only true self-avoiding walk)

        double resetrate;           ///< reset rate (only resetting random walk)
        double shift;               ///< shift before reset possible (only resetting random walk with shift)

        double gamma;               ///< direction change probability (only run-and-tumble walk)
        double total_length;        ///< total length of the walk (only run-and-tumble walk, fixed t)

        int width;                  ///< side length of a periodic field (only Scent walks)
        int tas;                    ///< lifetime of the scent (only Scent walks)
        agent_start_t agent_start;  ///< start configuration of the agents (only Scent walks)

        double lnf_min;             ///< minimum refinement factor up to which is simulated (only Wang Landau type simulations)
        double flatness_criterion;  ///< flatness criterion to be used (only Wang Landau type simulations)

        bool onlyBounds;            ///< output only the maximum and minimum of the observable and exit
        bool onlyCenters;           ///< output only the centers of the Wang Landau bins and exit
        bool onlyLERWExample;       ///< save a LERW with visualized erased loops and exit
        bool onlyChangeExample;     ///< save a picture of a random change and exit
        bool onlySingleChangeExample;///< save n pictures of random changes and exit
        bool onlyPivotExample;      ///< save a picture of a pivot step and exit
        bool onlyPTTemperatures;    ///< estimate good temperatures for parallel tempering and exit

        std::string text;           ///< the full command used to start this program
};

#endif
