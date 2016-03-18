#pragma once

#include <vector>
#include <algorithm>

#include "ConvexHull.hpp"

class ConvexHullAndrew : public ConvexHull
{
    public:
        ConvexHullAndrew(const std::vector<Step>& interiorPoints);
        virtual ~ConvexHullAndrew();

        virtual double A() const;
        virtual double L() const;

        virtual const std::vector<Step>& hullPoints() const;

        // void movePoint();

    protected:

};
