#include "ConvexHullAndrew.hpp"

// inspired by https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain#C.2B.2B

// 2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
// Returns a positive value, if OAB makes a counter-clockwise turn,
// negative for clockwise turn, and zero if the points are collinear.
double cross(const Step &O, const Step &A, const Step &B)
{
    return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
}

ConvexHullAndrew::ConvexHullAndrew(const std::vector<Step>& points, bool akl)
            : ConvexHull(points, akl)
{
    // create convex hull with graham scan, and akl heuristic (?)
    // graham scan does only work in a plane
    if(d != 2)
    {
        log<LOG_ERROR>("Andrew Monotone Chain does only work in a plane (d=2), the data is d =") << d;
        throw std::invalid_argument("Andrew Monotone Chain does only work in a plane (d=2)");
    }

    int k = 0;
    hullPoints_ = std::vector<Step>(2*n);

    std::sort(interiorPoints.begin(), interiorPoints.end());

    // Build lower hull
    for(int i = 0; i < n; ++i)
    {
        while (k >= 2 && cross(hullPoints_[k-2], hullPoints_[k-1], interiorPoints[i]) <= 0)
            k--;
        hullPoints_[k++] = interiorPoints[i];
    }

    // Build upper hull
    for (int i = n-2, t = k+1; i >= 0; i--)
    {
        while (k >= t && cross(hullPoints_[k-2], hullPoints_[k-1], interiorPoints[i]) <= 0)
            k--;
        hullPoints_[k++] = interiorPoints[i];
    }

    // last point equals first, this makes calculation of A and L easier
    hullPoints_.resize(k);
}

ConvexHullAndrew::~ConvexHullAndrew()
{
}

const std::vector<Step>& ConvexHullAndrew::hullPoints() const
{
    log<LOG_TOO_MUCH>("Convex Hull") << hullPoints_;
    return hullPoints_;
}

double ConvexHullAndrew::A() const
{
    // calculate Area in 2d -- since the algorithm only works for d=2
    double a = 0;
    for(int i=0; i<hullPoints_.size()-1; ++i)
        a += (hullPoints_[i].x() - hullPoints_[i+1].x())
            * (hullPoints_[i].y() + hullPoints_[i+1].y());
    return a/2;
}

double ConvexHullAndrew::L() const
{
    // calculate circumference in 2d -- since the algorithm only works for d=2
    double l = 0;
    for(int i=0; i<hullPoints_.size()-1; ++i)
        l += sqrt(
            std::pow(hullPoints_[i].x() - hullPoints_[i+1].x(),2)
            + std::pow(hullPoints_[i].y() - hullPoints_[i+1].y(), 2)
            );
    return l;
}
