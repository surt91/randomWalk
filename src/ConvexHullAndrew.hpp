#pragma once

#include <vector>
#include <algorithm>

#include "ConvexHull2D.hpp"

template <class T>
class ConvexHullAndrew : public ConvexHull2D<T>
{
    public:
        ConvexHullAndrew<T>(const std::vector<Step<T>>& interiorPoints, bool akl)
            : ConvexHull2D<T>(interiorPoints, akl)
        {
            if(this->d != 2)
            {
                LOG(LOG_ERROR) << "Andrew Monotone Chain does only work in a plane (d=2), the data is d = " << this->d;
                throw std::invalid_argument("Andrew Monotone Chain does only work in a plane (d=2)");
            }

            int k = 0;
            this->hullPoints_ = std::vector<Step<T>>(2*this->n);

            std::sort(this->interiorPoints.begin(), this->interiorPoints.end());

            // Build lower hull
            for(int i = 0; i < this->n; ++i)
            {
                while (k >= 2 && this->cross2d_z(this->hullPoints_[k-2], this->hullPoints_[k-1], this->interiorPoints[i]) <= 0)
                    k--;
                this->hullPoints_[k++] = this->interiorPoints[i];
            }

            // Build upper hull
            for (int i = this->n-2, t = k+1; i >= 0; i--)
            {
                while (k >= t && this->cross2d_z(this->hullPoints_[k-2], this->hullPoints_[k-1], this->interiorPoints[i]) <= 0)
                    k--;
                this->hullPoints_[k++] = this->interiorPoints[i];
            }

            // last point equals first, this makes calculation of A and L easier
            this->hullPoints_.resize(k);
        }
};
