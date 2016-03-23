#pragma once

#include <vector>
#include <algorithm>
#include <unordered_set>

#include "ConvexHull2D.hpp"

class ConvexHullJarvis : public ConvexHull2D
{
    public:
        ConvexHullJarvis(const std::vector<Step>& interiorPoints, bool akl);
        virtual ~ConvexHullJarvis();

    protected:

};
