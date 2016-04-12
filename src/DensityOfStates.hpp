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

        std::string binCentersString();
        std::string dataString();

        friend std::ostream& operator<<(std::ostream& os, const DensityOfStates &obj);
        DensityOfStates& operator+=(const DensityOfStates &other);

    protected:
        int bins;
        double lower;
        double upper;
        double binwidth;

        double fail;
        std::vector<double> data;

        bool checkBounds(double value);
};
