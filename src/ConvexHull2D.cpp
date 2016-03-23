#include "ConvexHull2D.hpp"

ConvexHull2D::ConvexHull2D(const std::vector<Step>& points, bool akl)
            : ConvexHull(points, akl)
{
    if(d != 2)
        throw std::invalid_argument("Only implemented for d=2");

    if(akl)
        preprocessAklToussaint();

    // mind that first and last entry of hullPoints_ need to be the same
}

ConvexHull2D::~ConvexHull2D()
{
}

const std::vector<Step>& ConvexHull2D::hullPoints() const
{
    Logger(LOG_TOO_MUCH) << "Convex Hull: " << hullPoints_;
    return hullPoints_;
}

double ConvexHull2D::A() const
{
    // calculate Area in 2d -- since the algorithm only works for d=2
    double a = 0;
    for(int i=0; i<hullPoints_.size()-1; ++i)
        a += (hullPoints_[i].x() - hullPoints_[i+1].x())
            * (hullPoints_[i].y() + hullPoints_[i+1].y());
    return a/2;
}

double ConvexHull2D::L() const
{
    // calculate circumference in 2d -- since the algorithm only works for d=2
    double l = 0;
    for(int i=0; i<hullPoints_.size()-1; ++i)
        l += sqrt(
            std::pow(hullPoints_[i].x() - hullPoints_[i+1].x(), 2)
            + std::pow(hullPoints_[i].y() - hullPoints_[i+1].y(), 2)
            );
    return l;
}

// special for d=2
int ConvexHull2D::side(const Step& p1, const Step& p2, const Step& p)
{
    return (p2.y() - p1.y())*(p.x() - p1.x()) + (-p2.x() + p1.x())*(p.y() - p1.y());
}

// sequence of the points matters, must be counterclockwise
bool ConvexHull2D::pointInTriangle(const Step& p1, const Step& p2, const Step& p3, const Step& p)
{
    bool checkSide1 = side(p1, p2, p) >= 0;
    bool checkSide2 = side(p2, p3, p) >= 0;
    bool checkSide3 = side(p3, p1, p) >= 0;
    return checkSide1 && checkSide2 && checkSide3;
}

// sequence of the points matters, must be counterclockwise
bool ConvexHull2D::pointInQuadrilateral(const Step& p1, const Step& p2, const Step& p3, const Step& p4, const Step& p)
{
    bool checkSide1 = side(p1, p2, p) >= 0;
    bool checkSide2 = side(p2, p3, p) >= 0;
    bool checkSide3 = side(p3, p4, p) >= 0;
    bool checkSide4 = side(p4, p1, p) >= 0;
    return checkSide1 && checkSide2 && checkSide3 && checkSide4;
}

// 2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
// Returns a positive value, if OAB makes a counter-clockwise turn,
// negative for clockwise turn, and zero if the points are collinear.
int ConvexHull2D::cross2d_z(const Step &O, const Step &A, const Step &B)
{
    return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
}


// deletes points from interior points according to the Akl Toussaint heuristic
void ConvexHull2D::preprocessAklToussaint()
{
    // could be generalized longterm:
    // - higher dimensions
    // - more than 4 points


    // find points with min/max x/y (/z/w/...)
    std::vector<Step> min(d, interiorPoints[0]);
    std::vector<Step> max(d, interiorPoints[0]);
    for(const Step& i : interiorPoints)
        for(int j=0; j<d; ++j)
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
    std::vector<Step> pointSelection;
    for(const Step& i : interiorPoints)
    {
        if(!pointInQuadrilateral(min[0], max[1], max[0], min[1], i))
            pointSelection.push_back(i);
    }

    // Also make sure that the min/max points are still considered
    for(int i=0; i<d; ++i)
    {
        pointSelection.push_back(min[i]);
        pointSelection.push_back(max[i]);
    }
    Logger(LOG_DEBUG) << "Akl Toussaint killed: "
            << (n - pointSelection.size()) << "/" << n
            << " ("  << std::setprecision(2) << ((double) (n - pointSelection.size()) / n * 100) << "%)";

    interiorPoints = std::move(pointSelection);
    n = interiorPoints.size();
}
