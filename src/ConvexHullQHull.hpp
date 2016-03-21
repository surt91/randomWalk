#pragma once

#include "qhull/src/libqhullcpp/Qhull.h"
#include "qhull/src/libqhullcpp/QhullVertex.h"
#include "qhull/src/libqhullcpp/QhullFacetList.h"
#include "qhull/src/libqhullcpp/QhullVertexSet.h"

#include "ConvexHull.hpp"

class ConvexHullQHull : public ConvexHull
{
    public:
        ConvexHullQHull(const std::vector<Step>& interiorPoints, bool akl);
        virtual ~ConvexHullQHull();

        virtual double A() const;
        virtual double L() const;

        virtual const std::vector<Step>& hullPoints() const;

        virtual std::vector<std::vector<Step>> hullFacets() const;

        // void movePoint();

    protected:
        std::unique_ptr<orgQhull::Qhull> qhull;
        double *coords;
};
