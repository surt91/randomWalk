#pragma once

#include <vector>
#include <algorithm>

#include "ConvexHull2D.hpp"

template <class T>
class ConvexHullAndrew : public ConvexHull2D<T>
{
    public:
        ConvexHullAndrew<T>(const std::vector<Step<T>> &interiorPoints, bool akl);
};

template <class T>
ConvexHullAndrew<T>::ConvexHullAndrew(const std::vector<Step<T>> &interiorPoints, bool akl)
    : ConvexHull2D<T>(interiorPoints, akl)
{
    if(this->d != 2)
    {
        LOG(LOG_ERROR) << "Andrew Monotone Chain does only work in a plane (d=2), the data is d = " << this->d;
        throw std::invalid_argument("Andrew Monotone Chain does only work in a plane (d=2)");
    }

    int k = 0;
    // we need to sort the points, hence we make a copy
    if(!akl)
        this->pointSelection = interiorPoints;

    this->hullPoints_ = std::vector<Step<T>>(2*this->n);

    std::sort(this->pointSelection.begin(), this->pointSelection.end());

    // Build lower hull
    for(int i = 0; i < this->n; ++i)
    {
        while (k >= 2 && cross2d_z(this->hullPoints_[k-2], this->hullPoints_[k-1], this->pointSelection[i]) <= 0)
            k--;
        this->hullPoints_[k++] = this->pointSelection[i];
    }

    // Build upper hull
    for (int i = this->n-2, t = k+1; i >= 0; i--)
    {
        while (k >= t && cross2d_z(this->hullPoints_[k-2], this->hullPoints_[k-1], this->pointSelection[i]) <= 0)
            k--;
        this->hullPoints_[k++] = this->pointSelection[i];
    }

    // last point equals first, this makes calculation of A and L easier
    this->hullPoints_.resize(k);
}
