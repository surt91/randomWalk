#pragma once

#include <Qhull.h>
#include <QhullVertex.h>
#include <QhullFacetList.h>
#include <QhullVertexSet.h>
#include <QhullError.h>

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

        double m_A;
        double m_L;
};
