#pragma once

#include <vector>

#include "Logging.hpp"

class DensityOfStates
{
    public:
        DensityOfStates(int bins, double lower, double upper);
        double& operator[](double value);
        void multiply(double value, double factor);

    protected:
        int bins;
        double lower;
        double upper;
        double fail;
        std::vector<double> data;

        bool checkBounds(double value);
};
