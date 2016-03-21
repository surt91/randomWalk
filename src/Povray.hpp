#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdexcept>

#include "Logging.hpp"

class Povray
{
    public:
        Povray(const std::string filename);

        void box(const double x, const double y, const double z, const double dx, const double dy, const double dz);
        void polyline(const std::vector<std::vector<double>> points);

        double stroke;

        void save();

    private:
        std::string filename;
        std::stringstream buffer;
        std::string header;
};
