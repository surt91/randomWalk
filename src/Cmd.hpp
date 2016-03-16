#pragma once

#include <string>
#include <sstream>

#include <tclap/CmdLine.h>

#include "Logging.hpp"

class Cmd
{
    public:
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

        bool benchmark;
};
