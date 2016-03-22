#include "ConvexHull.hpp"


ConvexHull::ConvexHull(const std::vector<Step>& interiorPoints, bool akl)
            : interiorPoints(interiorPoints),
              n(interiorPoints.size()),
              d(interiorPoints[0].d())
{
    if(akl)
        preprocessAklToussaint();
}

ConvexHull::~ConvexHull()
{
}

// special for d=2
double side(const Step& p1, const Step& p2, const Step& p)
{
    return (p2.y() - p1.y())*(p.x() - p1.x()) + (-p2.x() + p1.x())*(p.y() - p1.y());
}

// sequence of the points matters, must be counterclockwise
bool pointInQuadrilateral(const Step& p1, const Step& p2, const Step& p3, const Step& p4, const Step& p)
{
    bool checkSide1 = side(p1, p2, p) >= 0;
    bool checkSide2 = side(p2, p3, p) >= 0;
    bool checkSide3 = side(p3, p4, p) >= 0;
    bool checkSide4 = side(p4, p1, p) >= 0;
    return checkSide1 && checkSide2 && checkSide3 && checkSide4;
}

// deletes points from interior points according to the Akl Toussaint heuristic
void ConvexHull::preprocessAklToussaint()
{
    // could be generalized longterm:
    // - higher dimensions
    // - more than 4 points
    if(d != 2)
    {
        throw std::invalid_argument("Akl Toussaint Heuristic currently only implemented for d=2");
    }

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
    Logger(LOG_DEBUG) << "Akl Toussaint killed: " << (n - pointSelection.size());

    interiorPoints = std::move(pointSelection);
    n = interiorPoints.size();
}

std::vector<std::vector<Step>> ConvexHull::hullFacets() const
{
    throw std::invalid_argument("hull facets for this algorithm not implemented");
}
