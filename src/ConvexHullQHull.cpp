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
    if(d > 3)
        throw std::invalid_argument("facets not well thought through for d>=4");

    orgQhull::QhullFacetList fl = qhull->facetList();
    std::vector<std::vector<Step>> facets;

    Logger(LOG_INFO) << "Facet count: " << qhull->facetCount();

    for(const auto &f : fl)
    {
        std::vector<Step> facet;

        for(const auto v : f.vertices())
        {
            auto coord = v.point().coordinates();
            std::vector<int> s(coord, coord+d);

            facet.push_back(Step(s));
        }
        Logger(LOG_TOO_MUCH) << facet;

        if(!f.isSimplicial())
        {
            // divide the (hyper)polygon into simplical facets (eg triangles)
            // first order them clockwise (or counterclockwise)
            // - calculate a normal of the first 3 points (cross(a-c, b-c))
            // - calculate the normal of points 2 to 4 (cross(b-d, c-d))
            // - if the dot product of the normals is positive,
            //      they point in the same direction -> everything is fine
            //   else 4 and 3 need to be swapped
            // repeat til the end

            // points will not be collinear, thanks to qhull
            Step normal = cross(facet[0]-facet[2], facet[1]-facet[2]);
            const int num = facet.size();
            for(int i=1; i<num; ++i)
            {
                Step normal_tmp = cross(facet[i]-facet[(i+2)%num], facet[(i+1)%num]-facet[(i+2)%num]);
                if(dot(normal, normal_tmp) < 0)
                {
                    auto tmp = facet[(i+1)%num];
                    facet[(i+1)%num] = facet[(i+2)%num];
                    facet[(i+2)%num] = tmp;
                }
            }
            Logger(LOG_TOO_MUCH) << "reordered " << facet;

            // finally splitting the facet into triangles
            Logger(LOG_TOO_MUCH) << "subdivide to: ";
            for(int i=0; i<=facet.size() - d; i+=d)
            {
                std::vector<Step> simplex(facet.begin()+i, facet.begin()+i+d);
                Logger(LOG_TOO_MUCH) << simplex;
                facet.push_back(facet[i]);
                facet.push_back(facet[i+d-1]); // not sure for d>3
                facets.push_back(simplex);
            }
        }
        else
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
