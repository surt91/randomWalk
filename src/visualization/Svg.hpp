#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>

#include "../Logging.hpp"

/** Class to easily create 2d pictures in svg format.
 */
class SVG
{
    public:
        SVG(const std::string &filename, const double scale=1.0);

        void circle(const double x, const double y, const int filled);
        void line(const double x1, const double x2, const double y1, const double y2, const std::string color=std::string("black"));
        void polyline(const std::vector<std::vector<double>> points, const bool closed=false, const std::string color=std::string("black"));

        void text(const double x, const double y, const std::string &t, const std::string color=std::string("black"));

        double radius;
        double stroke;

        void save();

        void setScale(const double scale);
        void setGeometry(const double min, const double max, const bool border=false);
        void setGeometry(const double min_x, const double min_y, const double max_x, const double max_y, const bool border=false);

    private:
        double scale;
        std::string filename;
        std::stringstream buffer;
        std::string header;
};
