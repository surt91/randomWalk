#pragma once

#include <vector>
#include <sstream>
#include <string>

#include "Logging.hpp"

class DensityOfStates
{
    public:
        DensityOfStates(int bins, double lower, double upper);
        double& operator[](double value);
        void reset();

        bool checkBounds(double value);

        std::string binCentersString();
        std::string dataString();

        friend std::ostream& operator<<(std::ostream& os, const DensityOfStates &obj);
        DensityOfStates& operator+=(const DensityOfStates &other);

    protected:
        const int bins;
        const double lower;
        const double upper;
        const double binwidth;

        double fail;
        std::vector<double> data;
};
