#pragma once

#include "ConvexHull.hpp"

template <class T>
class ConvexHull2D : public ConvexHull<T>
{
    // templates are hard: http://stackoverflow.com/a/6592617/1698412
    public:
        ConvexHull2D<T>(const std::vector<Step<T>>& interiorPoints, bool akl)
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

        virtual double A() const
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

        virtual double L() const
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

        virtual const std::vector<Step<T>>& hullPoints() const
        {
            LOG(LOG_TOO_MUCH) << "Convex Hull: " << this->hullPoints_;
            return this->hullPoints_;
        }

        // 2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
        // Returns a positive value, if OAB makes a counter-clockwise turn,
        // negative for clockwise turn, and zero if the points are collinear.
        static int cross2d_z(const Step<T>& O, const Step<T>& A, const Step<T>& B)
        {
            return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
        }

        // sequence of the points matters, must be counterclockwise
        static bool pointInTriangle(const Step<T>& p1, const Step<T>& p2, const Step<T>& p3, const Step<T>& p)
        {
            bool checkSide1 = side(p1, p2, p) >= 0;
            bool checkSide2 = side(p2, p3, p) >= 0;
            bool checkSide3 = side(p3, p1, p) >= 0;
            return checkSide1 && checkSide2 && checkSide3;
        }

        // sequence of the points matters, must be counterclockwise
        static bool pointInQuadrilateral(const Step<T>& p1, const Step<T>& p2, const Step<T>& p3, const Step<T>& p4, const Step<T>& p)
        {
            bool checkSide1 = side(p1, p2, p) >= 0;
            bool checkSide2 = side(p2, p3, p) >= 0;
            bool checkSide3 = side(p3, p4, p) >= 0;
            bool checkSide4 = side(p4, p1, p) >= 0;
            return checkSide1 && checkSide2 && checkSide3 && checkSide4;
        }

        // sequence of the points matters, must be counterclockwise
        static bool pointInPolygon(const std::vector<Step<T>>& poly, const Step<T>& p)
        {
            bool inside = side(poly[poly.size()-1], poly[0], p) >= 0;
            for(size_t i=1; i<poly.size(); ++i)
                inside = inside && (side(poly[i-1], poly[i], p) >= 0);

            return inside;
        }

    protected:
        // special for d=2
        static int side(const Step<T>& p1, const Step<T>& p2, const Step<T>& p)
        {
            return (p2.y() - p1.y())*(p.x() - p1.x()) + (-p2.x() + p1.x())*(p.y() - p1.y());
        }

        // deletes points from interior points according to the Akl Toussaint heuristic
        virtual void preprocessAklToussaint()
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
            std::vector<Step<T>> pointSelection;
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
            LOG(LOG_DEBUG) << "Akl Toussaint killed: "
                    << (this->n - pointSelection.size()) << "/" << this->n
                    << " ("  << std::setprecision(2) << ((double) (this->n - pointSelection.size()) / this->n * 100) << "%)";

            this->interiorPoints = std::move(pointSelection);
            this->n = this->interiorPoints.size();
        }
};
