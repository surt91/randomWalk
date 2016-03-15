#pragma once

#include <set>
#include <memory>

#include "qhull/src/libqhullcpp/Qhull.h"
#include "qhull/src/libqhullcpp/QhullVertex.h"
#include "qhull/src/libqhullcpp/QhullFacetList.h"

#include "Step.hpp"

class ConvexHull
{
    public:
        ConvexHull(const std::vector<Step>& interiorPoints);
        ~ConvexHull();

        double A() const;
        double L() const;

        const std::vector<Step> hullPoints() const;

        // void movePoint();

    protected:
        std::unique_ptr<orgQhull::Qhull> qhull;
        double *coords;
        std::vector<Step> interiorPoints;
        int n;
        int d;
};
