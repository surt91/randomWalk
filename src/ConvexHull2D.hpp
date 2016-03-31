#pragma once

#include "ConvexHull.hpp"

class ConvexHull2D : public ConvexHull
{
    public:
        ConvexHull2D(const std::vector<Step>& interiorPoints, bool akl);
        virtual ~ConvexHull2D();

        virtual double A() const;
        virtual double L() const;

        virtual const std::vector<Step>& hullPoints() const;
        // void movePoint();

        static int cross2d_z(const Step &O, const Step &A, const Step &B);
        static bool pointInTriangle(const Step& p1, const Step& p2, const Step& p3, const Step& p);
        static bool pointInQuadrilateral(const Step& p1, const Step& p2, const Step& p3, const Step& p4, const Step& p);
        static bool pointInPolygon(const std::vector<Step>& poly, const Step& p);

    protected:
        static int side(const Step& p1, const Step& p2, const Step& p);

        virtual void preprocessAklToussaint();
};
