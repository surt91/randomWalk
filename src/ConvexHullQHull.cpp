#include "ConvexHullQHull.hpp"

// http://www.boost.org/doc/libs/1_56_0/libs/geometry/doc/html/geometry/reference/algorithms/convex_hull.html
// http://www.boost.org/doc/libs/1_57_0/libs/geometry/doc/html/geometry/reference/models/model_multi_point.html

ConvexHullQHull::ConvexHullQHull(const std::vector<Step>& points, bool akl)
            : ConvexHull(points, akl)
{
    coords = new double[n*d];
    for(int i=0; i<n; ++i)
        for(int j=0; j<d; ++j)
            coords[i*d + j] = interiorPoints[i][j];

    // comment, dimension, count, coordinates[], command
    qhull = std::unique_ptr<orgQhull::Qhull>(new orgQhull::Qhull("", d, n, coords, ""));
}

ConvexHullQHull::~ConvexHullQHull()
{
    delete[] coords;
}

const std::vector<Step>& ConvexHullQHull::hullPoints() const
{
    if(hullPoints_.size())
        return hullPoints_;

    orgQhull::QhullVertexList vl = qhull->vertexList();

    for(const auto &v : vl)
    {
        auto coord = v.point().coordinates();
        std::vector<int> s(coord, coord+d);
        hullPoints_.push_back(Step(s));
    }

    if(d==2) // for 2D we can order the points clockwise
    {
        std::sort(hullPoints_.begin(), hullPoints_.end(),
            [](Step const & a, Step const & b) -> bool
            { return a.angle() < b.angle(); } );
    }

    return hullPoints_;
}

std::vector<std::vector<Step>> ConvexHullQHull::hullFacets() const
{
    if(d < 3)
        throw std::invalid_argument("facets only implemented for d >= 3");

    orgQhull::QhullFacetList fl = qhull->facetList();
    std::vector<std::vector<Step>> facets;

    Logger(LOG_INFO) << "Facet count:" << qhull->facetCount();

    for(const auto &f : fl)
    {
        std::vector<Step> facet;
        for(const auto v : f.vertices())
        {
            auto coord = v.point().coordinates();
            std::vector<int> s(coord, coord+d);

            if(!f.isSimplicial)
            {
                // divide the (hyper)polygon into simplical facets (eg triangles)
                facet.push_back(Step(s));
            }
            else
                facet.push_back(Step(s));
        }
        Logger(LOG_TOO_MUCH) << facet;
        facets.push_back(facet);
    }

    return facets;
}

double ConvexHullQHull::A() const
{
    return qhull->volume();
}

double ConvexHullQHull::L() const
{
    return qhull->area();
}
