#include "ConvexHull.hpp"

// http://www.boost.org/doc/libs/1_56_0/libs/geometry/doc/html/geometry/reference/algorithms/convex_hull.html
// http://www.boost.org/doc/libs/1_57_0/libs/geometry/doc/html/geometry/reference/models/model_multi_point.html

ConvexHull::ConvexHull(const std::vector<Step>& interiorPoints)
            : interiorPoints(interiorPoints),
              n(interiorPoints.size()),
              d(interiorPoints[0].d())
{
    coords = new double[n*d];
    for(int i=0; i<n; ++i)
    {
        for(int j=0; j<d; ++j)
            coords[i*d + j] = interiorPoints[i][j];
    }

    // comment, dimension, count, coordinates[], command
    qhull = std::unique_ptr<orgQhull::Qhull>(new orgQhull::Qhull("", d, n, coords, ""));
}

ConvexHull::~ConvexHull()
{
    delete[] coords;
}

const std::vector<Step> ConvexHull::hullPoints() const
{
    std::vector<Step> out;
    orgQhull::QhullVertexList vl = qhull->vertexList();

    for(auto v : vl)
    {
        auto coord = v.point().coordinates();
        std::vector<int> s(d);
        for(int i=0; i<d; ++i)
            s[i] = coord[i];
        out.push_back(Step(s));
    }

    if(d==2) // for 2D we can order the points clockwise
    {
        std::sort(out.begin(), out.end(),
            [](Step const & a, Step const & b) -> bool
            { return a.angle() < b.angle(); } );
    }

    return out;
}

double ConvexHull::A() const
{
    return qhull->volume();
}

double ConvexHull::L() const
{
    return qhull->area();
}
