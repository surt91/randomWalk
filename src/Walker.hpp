#pragma once

#include <iostream>

#include "Svg.hpp"
#include "Step.hpp"
#include "ConvexHull.hpp"

/* Class Walker
 *
 * Saves a vector of random numbers [0,1] and generates a random walk
 * on demand.
 *
 * Also exhibits functions to change random numbers in that
 * vector and convinience functions to calculate the convex hull
 * of the walk.
 * */
class Walker
{
    public:
        Walker(int d, std::vector<double> random_numbers)
            : numSteps(random_numbers.size()),
              d(d), 
              random_numbers(random_numbers)
        {};

        const Step& position() const;

        void appendRN();

        void rnChange(const int idx, const double other);

        ConvexHull convexHull() const;

        const std::vector<Step> points() const;
        const std::vector<Step> steps() const;

        void print() const;
        void svg(const std::string filename, const bool with_hull=false) const;

    protected:
        int numSteps;
        int d;
        std::vector<double> random_numbers;
};

