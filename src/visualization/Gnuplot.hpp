#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdexcept>

#include "../Logging.hpp"

/** Class to easily create 2d/3d plots with gnuplot.
 *
 * Especially of random walks and their hulls.
 */
class Gnuplot
{
    public:
        Gnuplot(const std::string &filename);

        void polyline(const std::vector<std::vector<double>> &points);
        void facet(const std::vector<double> &x, const std::vector<double> &y, const std::vector<double> &z);

        void save();

    private:
        std::string filename;
        std::string filename_animate;
        std::string filename_points;
        std::string filename_hull;
        std::stringstream buffer;
        std::stringstream buffer_animate;
        std::stringstream buffer_points;
        std::stringstream buffer_hull;
};
