#ifndef GNUPLOT3D_H
#define GNUPLOT3D_H

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdexcept>

#include "../Logging.hpp"

/** Class to easily create 3d plots with gnuplot.
 *
 * Especially of random walks and their hulls.
 */
class Gnuplot3D
{
    public:
        Gnuplot3D(const std::string &filename);

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
        std::stringstream buffer_hull;
        std::vector<std::string> points_list;

        int numLines;
};

#endif
