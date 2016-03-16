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
        int n;
        int seed;
        int type;
        int d;
};
