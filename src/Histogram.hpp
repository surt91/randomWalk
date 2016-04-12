#pragma once

#include <vector>
#include <algorithm>

#include "Logging.hpp"

class Histogram
{
    public:
        Histogram(int bins, double lower, double upper);
        int min() const;
        double mean() const;
        int sum() const;
        void add(double value);
        void reset();

        int operator[](int idx) const;

        friend std::ostream& operator<<(std::ostream& os, const Histogram &obj);

    protected:
        int bins;
        double lower;
        double upper;
        double binwidth;

        int cur_min;
        int total;

        std::vector<int> counts;
};
