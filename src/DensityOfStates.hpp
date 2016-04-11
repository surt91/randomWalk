#pragma once

#include <vector>
#include <unordered_set>

#include "Logging.hpp"

class DensityOfStates
{
    public:
        DensityOfStates(int bins, double lower, double upper);
        double& operator[](double value);
        void multiply(double value, double factor);

        friend std::ostream& operator<<(std::ostream& os, const DensityOfStates &obj);
        DensityOfStates& operator+=(const DensityOfStates &other);

    protected:
        int bins;
        double lower;
        double upper;
        double fail;
        std::vector<double> data;

        bool checkBounds(double value);
};
