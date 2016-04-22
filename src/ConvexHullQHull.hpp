#pragma once

#include <Qhull.h>
#include <QhullVertex.h>
#include <QhullFacetList.h>
#include <QhullVertexSet.h>
#include <QhullError.h>

#include "geometry.hpp"
#include "ConvexHull.hpp"

// TODO: use akl for d=2
template <class T>
class ConvexHullQHull : public ConvexHull<T>
{
    // templates are hard: http://stackoverflow.com/a/6592617/1698412
    private:
        using ConvexHull<T>::d;
        using ConvexHull<T>::n;
        using ConvexHull<T>::m_L;
        using ConvexHull<T>::m_A;
        using ConvexHull<T>::hullPoints_;

    public:
        ConvexHullQHull(const std::vector<Step<T>> &interiorPoints, bool akl);
        virtual ~ConvexHullQHull() {}

        virtual double A() const { return m_A; };
        virtual double L() const { return m_L; };

        virtual const std::vector<Step<T>>& hullPoints() const;
        virtual std::vector<std::vector<Step<T>>> hullFacets() const;

    protected:
        std::unique_ptr<orgQhull::Qhull> qhull;
        std::vector<double> coords;
        virtual void preprocessAklToussaint();
};

template <class T>
ConvexHullQHull<T>::ConvexHullQHull(const std::vector<Step<T>> &interiorPoints, bool akl)
    : ConvexHull<T>(interiorPoints, akl)
{
    // test, if points are fully dimensional
    // we need to do that first, since qhull seems to leak on exceptions
    int num_zeros = 0;
    int zero_axis = 0;
    std::string cmd("");
    for(int i=0; i<d; ++i)
    {
        int j = 0;
        while(j < n && interiorPoints[j][i] == 0)
            ++j;
        if(j == n)
        {
            ++num_zeros;
            zero_axis = i;
        }
    }

    if(num_zeros >= 2)
    {
        LOG(LOG_DEBUG) << "Two dimensions less than fully dimensional";
        m_L = 0;
        m_A = 0;
        return;
    }
    else if(num_zeros == 1)
    {
        LOG(LOG_DEBUG) << "Not full dimensional, strip one axis: " << zero_axis;
        // drop that dimension, see http://www.qhull.org/html/qh-optq.htm#Qb0
        cmd = "Qb"+std::to_string(zero_axis)+":0B"+std::to_string(zero_axis)+":0";

        if(d - num_zeros <= 1)
        {
            LOG(LOG_DEBUG) << "One dimensional";
            m_L = 2*n;
            m_A = 0;
            return;
        }
        --d;
    }

    coords = std::vector<double>(n*d);

    if(akl)
        preprocessAklToussaint();
    else
    {
        for(int i=0; i<n; ++i)
            for(int j=0; j<d; ++j)
                coords[i*d + j] = interiorPoints[i][j];
    }

    try
    {
        // comment, dimension, count, coordinates[], command
        qhull = std::unique_ptr<orgQhull::Qhull>(new orgQhull::Qhull("", d, n, coords.data(), cmd.c_str()));
        if(num_zeros == 1)
        {
            m_L = qhull->volume();
            m_A = 0;
        }
        else if(num_zeros == 0)
        {
            m_A = qhull->volume();
            m_L = qhull->area();
        }
    }
    catch(orgQhull::QhullError &e)
    {
        LOG(LOG_ERROR) << "Not full dimensional, but not catched!";
        LOG(LOG_TOO_MUCH) << e.what();
    }
}

// deletes points from interior points according to the Akl Toussaint heuristic
template <class T>
void ConvexHullQHull<T>::preprocessAklToussaint()
{
    if(this->d != 2)
        throw std::invalid_argument("Only implemented for d=2");

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
    int k = 0;
    for(const Step<T>& i : this->interiorPoints)
        if(!pointInQuadrilateral(min[0], max[1], max[0], min[1], i))
        {
            for(int j=0; j<this->d; ++j)
                coords[k*this->d + j] = i[j];
            ++k;
        }

    // Also make sure that the min/max points are still considered
    for(int i=0; i<this->d; ++i, k+=2)
        for(int j=0; j<this->d; ++j)
        {
            coords[k*this->d + j] = min[i][j];
            coords[(k+1)*this->d + j] = max[i][j];
        }

    LOG(LOG_TOO_MUCH) << "Akl Toussaint killed: "
            << (this->n - k) << "/" << this->n
            << " ("  << std::setprecision(2) << ((double) (this->n - k) / this->n * 100) << "%)";

    coords.resize(k*this->d);
    this->n = k;
}

template <class T>
const std::vector<Step<T>>& ConvexHullQHull<T>::hullPoints() const
{
    if(hullPoints_.size())
        return hullPoints_;

    orgQhull::QhullVertexList vl = qhull->vertexList();

    for(const auto &v : vl)
    {
        auto coord = v.point().coordinates();
        std::vector<T> s(coord, coord+d);
        hullPoints_.emplace_back(std::move(s));
    }

    if(d==2) // for 2D we can order the points clockwise
    {
        // FIXME: will not work for the point at (0, 0)
        std::sort(hullPoints_.begin(), hullPoints_.end(),
            [](const Step<T> &a, const Step<T> &b) -> bool
            { return a.angle() < b.angle(); } );
    }

    return hullPoints_;
}

template <class T>
std::vector<std::vector<Step<T>>> ConvexHullQHull<T>::hullFacets() const
{
    if(d < 3)
        throw std::invalid_argument("facets only implemented for d >= 3");
    if(d > 3)
        throw std::invalid_argument("facets not well thought through for d>=4");

    orgQhull::QhullFacetList fl = qhull->facetList();
    std::vector<std::vector<Step<T>>> facets;

    LOG(LOG_INFO) << "Facet count: " << qhull->facetCount();

    for(const auto &f : fl)
    {
        std::vector<Step<T>> facet;

        for(const auto v : f.vertices())
        {
            auto coord = v.point().coordinates();
            std::vector<T> s(coord, coord+d);

            facet.push_back(Step<T>(s));
        }
        LOG(LOG_TOO_MUCH) << facet;

        if(!f.isSimplicial())
        {
            // calculate the normal, and find the two most suitable axis (maximizing the dot product)
            // project the points onto the plane of the two axis, i, j
            // sort by stupid atan2

            // points will not be collinear, thanks to qhull
            Step<T> normal = cross(facet[0]-facet[2], facet[1]-facet[2]);
            double x = std::abs(dot(normal, Step<T>({1, 0, 0})));
            double y = std::abs(dot(normal, Step<T>({0, 1, 0})));
            double z = std::abs(dot(normal, Step<T>({0, 0, 1})));
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
            Step<T> inside({0, 0, 0});
            for(auto &v : facet)
                inside += v;
            inside /= facet.size();
            // FIXME inside is integer values and maybe outside of the facet
            // but probably does not harm since all other points are also integer

            LOG(LOG_TOO_MUCH) << "normal " << normal;
            LOG(LOG_TOO_MUCH) << "inside " << inside;
            LOG(LOG_TOO_MUCH) << "axes " << i << j;
            std::sort(facet.begin(), facet.end(),
                    [i,j, &inside](Step<T> const &a, Step<T> const &b) -> bool
                    { return (a-inside).angle(i, j) < (b-inside).angle(i, j); } );

            LOG(LOG_TOO_MUCH) << "reordered " << facet;

            // finally splitting the facet into triangles
            LOG(LOG_TOO_MUCH) << "subdivide to: ";
            for(size_t i=0; i<=facet.size() - d; i+=d)
            {
                std::vector<Step<T>> simplex(facet.begin()+i, facet.begin()+i+d);
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
