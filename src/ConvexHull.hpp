#pragma once

#include <set>
#include <memory>

#include "Logging.hpp"
#include "Step.hpp"

class ConvexHull
{
    public:
        ConvexHull(const std::vector<Step>& interiorPoints);
        virtual ~ConvexHull();

        virtual double A() const = 0;
        virtual double L() const = 0;

        virtual const std::vector<Step>& hullPoints() const = 0;

        // void movePoint();

    protected:
        std::vector<Step> interiorPoints;
        mutable std::vector<Step> hullPoints_;
        int n;
        int d;

        void preprocessAklToussaint();
};
