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

const std::vector<std::string> TYPE_LABEL = {
    "nan",
    "Random Walk",
    "Loop Erased Random Walk",
    "Self-Avoiding Random Walk"
};

class Cmd
{
    public:
        Cmd() {};
        Cmd(int argc, char** argv);

        std::string svg_path;
        std::string data_path;
        int steps;
        int seedRealization;
        int seedMC;
        int type;
        int d;
        int iterations;
        int theta;
        hull_algorithm_t chAlg;

        bool benchmark;
        double benchmark_A;
        double benchmark_L;
};
