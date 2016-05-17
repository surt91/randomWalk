#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdexcept>

#include "../Logging.hpp"

/** Class to easily create 2d plots with gnuplot.
 *
 * Especially of random walks and their hulls.
 */
class Gnuplot2D
{
    public:
        Gnuplot2D(const std::string &filename);

        void polyline(const std::vector<std::vector<double>> &points, bool hull=false);

        void save();

    private:
        std::string filename;
        std::string filename_points;
        std::string filename_hull;
        std::stringstream buffer;
        std::stringstream buffer_points;
        std::stringstream buffer_hull;
};
