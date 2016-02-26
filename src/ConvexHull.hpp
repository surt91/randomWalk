#pragma once

#include "Step.hpp"

class ConvexHull
{
    public:
        ConvexHull(const std::vector<Step>& interiorPoints)
            : interiorPoints(interiorPoints)
        {};

        double A() const;
        double L() const;

        const std::vector<Step> hullPoints() const;

        // void movePoint();

    protected:
        std::vector<Step> interiorPoints;
};
