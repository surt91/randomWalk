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

    try
    {
        // comment, dimension, count, coordinates[], command
        qhull = std::unique_ptr<orgQhull::Qhull>(new orgQhull::Qhull("", d, n, coords, ""));
        m_A = qhull->volume();
        m_L = qhull->area();
    }
    catch(orgQhull::QhullError &e)
    {
        // not full dimensional
        // discard one dimension, calculate again
        // this is easy since the points are on a lattice
        LOG(LOG_WARNING) << "Not full dimensional, strip one axis";
        LOG(LOG_TOO_MUCH) << e.what();

        std::vector<int> dimMap(d-1);
        for(int i=0; i<d; ++i)
        {
            int j = 0;
            while(j < n && interiorPoints[j][i] == 0)
                ++j;
            if(j == n)
            {
                for(int l=0, k=0; l<d; ++l)
                    if(l != i)
                        dimMap[k++] = l;
                break;
            }
        }

        LOG(LOG_TOO_MUCH) << "points: " << interiorPoints;
        LOG(LOG_DEBUG) << "axis left: " << dimMap;

        delete[] coords;
        --d;
        coords = new double[n*d];
        for(int i=0; i<n; ++i)
            for(int j=0; j<d; ++j)
                coords[i*d + j] = interiorPoints[i][dimMap[j]];

        // if it still does not work, L and A are 0
        try
        {
            qhull = std::unique_ptr<orgQhull::Qhull>(new orgQhull::Qhull("", d, n, coords, ""));
            m_L = qhull->volume();
            m_A = 0;
        }
        catch(orgQhull::QhullError &e)
        {
            LOG(LOG_WARNING) << "Two dimensions less than fully dimensional";
            m_L = 0;
            m_A = 0;
        }
    }
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
        hullPoints_.emplace_back(std::move(s));
    }

    if(d==2) // for 2D we can order the points clockwise
    {
        // FIXME: will not work for the point at (0, 0)
        std::sort(hullPoints_.begin(), hullPoints_.end(),
            [](const Step &a, const Step &b) -> bool
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

    LOG(LOG_INFO) << "Facet count: " << qhull->facetCount();

    for(const auto &f : fl)
    {
        std::vector<Step> facet;

        for(const auto v : f.vertices())
        {
            auto coord = v.point().coordinates();
            std::vector<int> s(coord, coord+d);

            facet.push_back(Step(s));
        }
        LOG(LOG_TOO_MUCH) << facet;

        if(!f.isSimplicial())
        {
            // calculate the normal, and find the two most suitable axis (maximizing the dot product)
            // project the points onto the plane of the two axis, i, j
            // sort by stupid atan2

            // points will not be collinear, thanks to qhull
            Step normal = cross(facet[0]-facet[2], facet[1]-facet[2]);
            double x = std::abs(dot(normal, Step({1, 0, 0})));
            double y = std::abs(dot(normal, Step({0, 1, 0})));
            double z = std::abs(dot(normal, Step({0, 0, 1})));
            int i, j;
            if(x >= y && x >= z)
            {
                i = 1; // y
                j = 2; // z
            }
            else if(y >= x && y >= z)
            {
                i = 0; // x
                j = 2; // z
            }
            else
            {
                i = 0; // x
                j = 1; // y
            }
            Step inside({0, 0, 0});
            for(auto &v : facet)
                inside += v;
            inside /= facet.size();
            // FIXME inside is integer values and maybe outside of the facet
            // but probably does not harm since all other points are also integer

            LOG(LOG_TOO_MUCH) << "normal " << normal;
            LOG(LOG_TOO_MUCH) << "inside " << inside;
            LOG(LOG_TOO_MUCH) << "axes " << i << j;
            std::sort(facet.begin(), facet.end(),
                    [i,j, &inside](Step const &a, Step const &b) -> bool
                    { return (a-inside).angle(i, j) < (b-inside).angle(i, j); } );

            LOG(LOG_TOO_MUCH) << "reordered " << facet;

            // finally splitting the facet into triangles
            LOG(LOG_TOO_MUCH) << "subdivide to: ";
            for(size_t i=0; i<=facet.size() - d; i+=d)
            {
                std::vector<Step> simplex(facet.begin()+i, facet.begin()+i+d);
                LOG(LOG_TOO_MUCH) << simplex;
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
    return m_A;
}

double ConvexHullQHull::L() const
{
    return m_L;
}
