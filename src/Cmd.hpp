#pragma once

#include <string>
#include <sstream>

#include <tclap/CmdLine.h>

#include "Logging.hpp"

enum hull_algorithm_t {
    CH_QHULL = 1,
    CH_QHULL_AKL,
    CH_ANDREWS,
    CH_ANDREWS_AKL,
    CH_GRAHAM,
    CH_GRAHAM_AKL,
    CH_JARVIS,
    CH_JARVIS_AKL,
    CH_CHAN,
    CH_CHAN_AKL
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
    WT_RANDOM_WALK = 1,
    WT_LOOP_ERASED_RANDOM_WALK,
    WT_SELF_AVOIDING_RANDOM_WALK
};

const std::vector<std::string> TYPE_LABEL = {
    "nan",
    "Random Walk",
    "Loop Erased Random Walk",
    "Self-Avoiding Random Walk"
};

enum wanted_observable_t {
    WO_SURFACE_AREA = 1,    // eg circumference in d=2
    WO_VOLUME              // eg area in d=2
};

const std::vector<std::string> WANTED_OBSERVABLE_LABEL = {
    "nan",
    "surface area (circumference in d=2)",
    "volume (area in d=2)"
};

class Cmd
{
    public:
        Cmd() {};
        Cmd(int argc, char** argv);

        std::string tmp_path;
        std::string data_path;
        std::string svg_path;
        std::string pov_path;
        int steps;
        int seedRealization;
        int seedMC;
        walk_type_t type;
        int d;
        int iterations;
        int theta;
        hull_algorithm_t chAlg;
        wanted_observable_t wantedObservable;

        bool benchmark;
        double benchmark_A;
        double benchmark_L;
};
