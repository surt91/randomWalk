#ifndef POVRAY_H
#define POVRAY_H

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdexcept>

#include "../Logging.hpp"

/** Class to easily create 3d renderings.
 *
 * Especially of random walks and their hulls.
 */
class Povray
{
    public:
        Povray(const std::string &filename);

        void box(const double x, const double y, const double z, const double dx, const double dy, const double dz);
        void polyline(const std::vector<std::vector<double>> &points);
        void facet(const std::vector<double> &x, const std::vector<double> &y, const std::vector<double> &z);

        double stroke;

        void save();

    private:
        std::string filename;
        std::stringstream buffer;
        std::string header;
};

#endif
