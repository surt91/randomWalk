#pragma once

#include <vector>
#include <algorithm>

#include "ConvexHull2D.hpp"

class ConvexHullAndrew : public ConvexHull2D
{
    public:
        ConvexHullAndrew(const std::vector<Step>& interiorPoints, bool akl);
        virtual ~ConvexHullAndrew();

    protected:

};
