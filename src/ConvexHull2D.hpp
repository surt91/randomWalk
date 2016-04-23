#pragma once

#include "geometry.hpp"
#include "ConvexHull.hpp"

template <class T>
class ConvexHull2D : public ConvexHull<T>
{
    // templates are hard: http://stackoverflow.com/a/6592617/1698412
    public:
        ConvexHull2D(const std::vector<Step<T>> &interiorPoints, bool akl);
        virtual ~ConvexHull2D() {}

        virtual double A() const;
        virtual double L() const;

        virtual const std::vector<Step<T>>& hullPoints() const;

    protected:
        virtual void preprocessAklToussaint();
        std::vector<Step<T>> pointSelection;
};

template <class T>
ConvexHull2D<T>::ConvexHull2D(const std::vector<Step<T>> &interiorPoints, bool akl)
    : ConvexHull<T>(interiorPoints, akl)
{
    if(this->d != 2)
        throw std::invalid_argument("Only implemented for d=2");

    if(akl)
        preprocessAklToussaint();

    // invalid value, so that the getter know, to calculate them
    this->m_A = -1;
    this->m_L = -1;

    // mind that first and last entry of hullPoints_ need to be the same
}

template <class T>
double ConvexHull2D<T>::A() const
{
    if(this->m_A < 0)
    {
        // calculate Area in 2d -- since the algorithm only works for d=2
        double a = 0;
        for(size_t i=0; i<this->hullPoints_.size()-1; ++i)
            a += (this->hullPoints_[i].x() - this->hullPoints_[i+1].x())
                * (this->hullPoints_[i].y() + this->hullPoints_[i+1].y());

        this->m_A = a/2;
    }

    return this->m_A;
}

template <class T>
double ConvexHull2D<T>::L() const
{
    if(this->m_L < 0)
    {
        // calculate circumference in 2d -- since the algorithm only works for d=2
        double l = 0;
        for(size_t i=0; i<this->hullPoints_.size()-1; ++i)
            l += sqrt(
                std::pow(this->hullPoints_[i].x() - this->hullPoints_[i+1].x(), 2)
                + std::pow(this->hullPoints_[i].y() - this->hullPoints_[i+1].y(), 2)
                );
        this->m_L = l;
    }

    return this->m_L;
}

template <class T>
const std::vector<Step<T>>& ConvexHull2D<T>::hullPoints() const
{
    LOG(LOG_TOO_MUCH) << "Convex Hull: " << this->hullPoints_;
    return this->hullPoints_;
}

// deletes points from interior points according to the Akl Toussaint heuristic
template <class T>
void ConvexHull2D<T>::preprocessAklToussaint()
{
    // could be generalized longterm:
    // - higher dimensions
    // - more than 4 points

    // find points with min/max x/y (/z/w/...)
    std::vector<Step<T>> min(this->d, this->interiorPoints[0]);
    std::vector<Step<T>> max(this->d, this->interiorPoints[0]);
    for(const Step<T>& i : this->interiorPoints)
        for(int j=0; j<this->d; ++j)
        {
            if(i.x(j) < min[j].x(j))
                min[j] = i;
            if(i.x(j) > max[j].x(j))
                max[j] = i;
        }

    // for d=3 this needs to be a volume instead of a polygon
    // find and delete points inside the quadriliteral
    // http://totologic.blogspot.de/2014/01/accurate-point-in-triangle-test.html
    // do this by building a new list of vertices outside
    for(const Step<T>& i : this->interiorPoints)
    {
        if(!pointInQuadrilateral(min[0], max[1], max[0], min[1], i))
            pointSelection.push_back(i);
    }

    // Also make sure that the min/max points are still considered
    for(int i=0; i<this->d; ++i)
    {
        pointSelection.push_back(min[i]);
        pointSelection.push_back(max[i]);
    }
    LOG(LOG_TOO_MUCH) << "Akl Toussaint killed: "
            << (this->n - pointSelection.size()) << "/" << this->n
            << " ("  << std::setprecision(2) << ((double) (this->n - pointSelection.size()) / this->n * 100) << "%)";

    this->n = pointSelection.size();
}
