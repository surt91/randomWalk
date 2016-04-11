#pragma once

#include <vector>
#include <algorithm>

#include "Logging.hpp"

class Histogram
{
    public:
        Histogram(int bins, double lower, double upper);
        int min();
        double mean();
        void add(double value);
        void reset();

        int operator[](int idx);

    protected:
        int bins;
        double lower;
        double upper;

        int cur_min;
        int total;

        std::vector<int> counts;
};
