#include "ConvexHullAndrew.hpp"

// inspired by https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain#C.2B.2B

ConvexHullAndrew::ConvexHullAndrew(const std::vector<Step>& points, bool akl)
            : ConvexHull2D(points, akl)
{
    if(d != 2)
    {
        LOG(LOG_ERROR) << "Andrew Monotone Chain does only work in a plane (d=2), the data is d = " << d;
        throw std::invalid_argument("Andrew Monotone Chain does only work in a plane (d=2)");
    }

    int k = 0;
    hullPoints_ = std::vector<Step>(2*n);

    std::sort(interiorPoints.begin(), interiorPoints.end());

    // Build lower hull
    for(int i = 0; i < n; ++i)
    {
        while (k >= 2 && cross2d_z(hullPoints_[k-2], hullPoints_[k-1], interiorPoints[i]) <= 0)
            k--;
        hullPoints_[k++] = interiorPoints[i];
    }

    // Build upper hull
    for (int i = n-2, t = k+1; i >= 0; i--)
    {
        while (k >= t && cross2d_z(hullPoints_[k-2], hullPoints_[k-1], interiorPoints[i]) <= 0)
            k--;
        hullPoints_[k++] = interiorPoints[i];
    }

    // last point equals first, this makes calculation of A and L easier
    hullPoints_.resize(k);
}

ConvexHullAndrew::~ConvexHullAndrew()
{
}
