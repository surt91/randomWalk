#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdexcept>

#include "../Logging.hpp"
#include "../stat/HistogramND.hpp"

/** Class to easily create 2d contour plots with gnuplot.
 */
class GnuplotContour
{
    public:
        GnuplotContour(const std::string &filename);

        void data(const std::vector<HistogramND> &histograms);

        void save();

    private:
        std::string filename;
        std::string filename_png;
        std::string filename_matrix;
        std::stringstream buffer;
        std::stringstream buffer_points;
};
