#pragma once

#include <set>
#include <memory>

#include "Step.hpp"

class ConvexHull
{
    public:
        ConvexHull(const std::vector<Step>& interiorPoints);
        virtual ~ConvexHull();

        virtual double A() const = 0;
        virtual double L() const = 0;

        virtual const std::vector<Step> hullPoints() const = 0;

        // void movePoint();

    protected:
        std::vector<Step> interiorPoints;
        int n;
        int d;
};
