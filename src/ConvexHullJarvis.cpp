#include "ConvexHullJarvis.hpp"

ConvexHullJarvis::ConvexHullJarvis(const std::vector<Step>& points, bool akl)
            : ConvexHull2D(points, akl)
{
    if(d > 3)
    {
        LOG(LOG_ERROR) << "Jarvis March does only work in d=2 and d=3, the data is d = " << d;
        throw std::invalid_argument("Jarvis March does only work in d=2 and d=3");
    }
    if(d != 2)
    {
        LOG(LOG_ERROR) << "Jarvis March is only implemented in d=2, the data is d = " << d;
        throw std::invalid_argument("Jarvis March is only implemented in d=2");
    }

    // Step < sorts by x value
    std::unordered_set<Step> candidate_points(interiorPoints.begin(), interiorPoints.end());
    Step p, p1;

    // find leftmost point
    p1 = std::min(interiorPoints);
    candidate_points.erase(p1);
    hullPoints_.push_back(p1);

    int hull_idx = 0;
    do
    {
        // Search for a point 'p' such that orientation(hull.last, i, p) is
        // counterclockwise for all points 'i'
        p = p1;

        for(auto it = candidate_points.begin(); it != candidate_points.end(); ++it)
        {
            int orientation = cross2d_z(hullPoints_[hull_idx], *it, p);
            if(orientation > 0)
                p = *it;
            else if(orientation == 0) // colinear
            {
                // take the one furthest away
                if((p-hullPoints_[hull_idx]).length() < (*it-hullPoints_[hull_idx]).length())
                    p = *it;
                // and delete all other
            }
        }

        hull_idx++;
        hullPoints_.push_back(p);

        // found, then elimiminate all points inside the triangle (hull[0], hull[i-1], hull[i])
        candidate_points.erase(p);

        for(auto it = candidate_points.begin(); it != candidate_points.end(); )
        {
            if(pointInTriangle(hullPoints_[0],
                               hullPoints_[hull_idx-1],
                               hullPoints_[hull_idx],
                               *it))
                it = candidate_points.erase(it);
            else
                ++it;
        }
    } while(p!=p1); // if we reach the first, we have finished
    // mind that first and last entry of hullPoints_ are the same
}

ConvexHullJarvis::~ConvexHullJarvis()
{
}
