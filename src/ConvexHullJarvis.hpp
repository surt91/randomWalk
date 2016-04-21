#pragma once

#include <vector>
#include <algorithm>
#include <unordered_set>

#include "ConvexHull2D.hpp"

template <class T>
class ConvexHullJarvis : public ConvexHull2D<T>
{
    public:
        ConvexHullJarvis<T>(const std::vector<Step<T>>& interiorPoints, bool akl);
};

template <class T>
ConvexHullJarvis<T>::ConvexHullJarvis(const std::vector<Step<T>>& interiorPoints, bool akl)
    : ConvexHull2D<T>(interiorPoints, akl)
{
    if(this->d > 3)
    {
        LOG(LOG_ERROR) << "Jarvis March does only work in d=2 and d=3, the data is d = " << this->d;
        throw std::invalid_argument("Jarvis March does only work in d=2 and d=3");
    }
    if(this->d != 2)
    {
        LOG(LOG_ERROR) << "Jarvis March is only implemented in d=2, the data is d = " << this->d;
        throw std::invalid_argument("Jarvis March is only implemented in d=2");
    }

    // Step < sorts by x value
    std::unordered_set<Step<T>> candidate_points(interiorPoints.begin(), interiorPoints.end());
    Step<T> p, p1;

    // find leftmost point
    p1 = std::min(interiorPoints);
    candidate_points.erase(p1);
    this->hullPoints_.push_back(p1);

    int hull_idx = 0;
    do
    {
        // Search for a point 'p' such that orientation(hull.last, i, p) is
        // counterclockwise for all points 'i'
        p = p1;

        for(auto it = candidate_points.begin(); it != candidate_points.end(); ++it)
        {
            int orientation = this->cross2d_z(this->hullPoints_[hull_idx], *it, p);
            if(orientation > 0)
                p = *it;
            else if(orientation == 0) // colinear
            {
                // take the one furthest away
                if((p-this->hullPoints_[hull_idx]).length() < (*it-this->hullPoints_[hull_idx]).length())
                    p = *it;
                // and delete all other
            }
        }

        hull_idx++;
        this->hullPoints_.push_back(p);

        // found, then elimiminate all points inside the triangle (hull[0], hull[i-1], hull[i])
        candidate_points.erase(p);

        for(auto it = candidate_points.begin(); it != candidate_points.end(); )
        {
            if(this->pointInTriangle(this->hullPoints_[0],
                               this->hullPoints_[hull_idx-1],
                               this->hullPoints_[hull_idx],
                               *it))
                it = candidate_points.erase(it);
            else
                ++it;
        }
    } while(p!=p1); // if we reach the first, we have finished
    // mind that first and last entry of hullPoints_ are the same
}
