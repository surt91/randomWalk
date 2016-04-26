#pragma once

#include <set>
#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_set>

#include <Qhull.h>
#include <QhullVertex.h>
#include <QhullFacetList.h>
#include <QhullVertexSet.h>
#include <QhullError.h>

#include "Logging.hpp"
#include "Step.hpp"
#include "geometry.hpp"

/** Class template for calculation of Convex Hulls.
 *
 * Uses an algorithm given in the constructor to calculate the convex
 * hull and some of its properties.
 *
 * \tparam T datatype of the Steps (integer or double)
 */
template <class T>
class ConvexHull
{
    protected:
        int n;
        int d;
        const hull_algorithm_t algorithm;

        mutable double m_A;
        mutable double m_L;

        const std::vector<Step<T>>& interiorPoints;
        mutable std::vector<Step<T>> hullPoints_;

    public:
        ConvexHull(const std::vector<Step<T>> &interiorPoints, hull_algorithm_t algorithm)
            : n(interiorPoints.size()),
              d(interiorPoints[0].d()),
              algorithm(algorithm),
              interiorPoints(interiorPoints)
        {
            m_L = -1.;
            m_A = -1.;
            run();
        }
        virtual ~ConvexHull() {}

        // observables
        double A() const;
        double L() const;
        //~ std::vector<T> max_extent();
        //~ double diameter();

        // hull
        const std::vector<Step<T>>& hullPoints() const;
        std::vector<std::vector<Step<T>>> hullFacets() const;

    protected:
        void run();

        // for QHull
        void runQhull();
        void preprocessAklToussaintQHull();

        std::unique_ptr<orgQhull::Qhull> qhull;
        std::vector<double> coords;
        void updateHullPoints() const;
        int countZerosAndUpdateCmd();
        std::string cmd;

        // for Andrews, and Jarvis
        void runAndrews();
        void runJarvis();
        void preprocessAklToussaint();

        std::vector<int> pointSelection;

};

template <class T>
void ConvexHull<T>::run()
{
    switch(algorithm)
    {
        case CH_QHULL_AKL:
            preprocessAklToussaintQHull();
        case CH_QHULL:
            runQhull();
            break;
        case CH_ANDREWS_AKL:
            preprocessAklToussaint();
        case CH_ANDREWS:
            runAndrews();
            break;
        case CH_JARVIS_AKL:
            preprocessAklToussaint();
        case CH_JARVIS:
            runJarvis();
            break;
        default:
            LOG(LOG_ERROR) << "Algorithm not implemented, yet: " << CH_LABEL[algorithm];
            throw std::invalid_argument("this is not implemented");
    }
}

template <class T>
double ConvexHull<T>::A() const
{
    if(m_A < 0)
    {
        if(d != 2)
            throw std::invalid_argument("volume calculation only implemented for d=2");
        // calculate Area in 2d -- since the algorithm only works for d=2
        double a = 0;
        for(size_t i=0; i<hullPoints_.size()-1; ++i)
            a += (hullPoints_[i].x() - hullPoints_[i+1].x())
                * (hullPoints_[i].y() + hullPoints_[i+1].y());

        m_A = a/2;
    }

    return m_A;
}

template <class T>
double ConvexHull<T>::L() const
{
    if(m_L < 0)
    {
        if(d != 2)
            throw std::invalid_argument("surface area calculation only implemented for d=2");
        // calculate circumference in 2d -- since the algorithm only works for d=2
        double l = 0;
        for(size_t i=0; i<hullPoints_.size()-1; ++i)
            l += sqrt(
                std::pow(hullPoints_[i].x() - hullPoints_[i+1].x(), 2)
                + std::pow(hullPoints_[i].y() - hullPoints_[i+1].y(), 2)
                );
        m_L = l;
    }

    return m_L;
}

template <class T>
const std::vector<Step<T>>& ConvexHull<T>::hullPoints() const
{
    if(hullPoints_.empty())
        updateHullPoints();
    LOG(LOG_TOO_MUCH) << "Convex Hull: " << hullPoints_;
    return hullPoints_;
}

/// deletes points from interior points according to the Akl Toussaint heuristic
template <class T>
void ConvexHull<T>::preprocessAklToussaint()
{
    // could be generalized longterm:
    // - higher dimensions
    // - more than 4 points
    pointSelection.reserve(n);

    // find points with min/max x/y (/z/w/...)
    std::vector<int> min(d, 0);
    std::vector<int> max(d, 0);
    for(int i=0; i<n; ++i)
        for(int j=0; j<d; ++j)
        {
            if(interiorPoints[i][j] < interiorPoints[min[j]][j])
                min[j] = i;
            if(interiorPoints[i][j] > interiorPoints[max[j]][j])
                max[j] = i;
        }

    // for d=3 this needs to be a volume instead of a polygon
    // find and delete points inside the quadriliteral
    // http://totologic.blogspot.de/2014/01/accurate-point-in-triangle-test.html
    // do this by building a new list of vertices outside
    //~ for(const Step<T>& i : interiorPoints)
    for(int i=0; i<n; ++i)
        if(!pointInQuadrilateral(interiorPoints[min[0]], interiorPoints[max[1]], interiorPoints[max[0]], interiorPoints[min[1]], interiorPoints[i]))
            pointSelection.emplace_back(i);

    // Also make sure that the min/max points are still considered
    for(int i=0; i<d; ++i)
    {
        pointSelection.emplace_back(min[i]);
        pointSelection.emplace_back(max[i]);
    }
    LOG(LOG_TOO_MUCH) << "Akl Toussaint killed: "
            << (n - pointSelection.size()) << "/" << n
            << " ("  << std::setprecision(2) << ((double) (n - pointSelection.size()) / n * 100) << "%)";

    n = pointSelection.size();
}

/// deletes points from interior points according to the Akl Toussaint heuristic
template <class T>
void ConvexHull<T>::preprocessAklToussaintQHull()
{
    if(d != 2)
        throw std::invalid_argument("Only implemented for d=2");

    coords.reserve(d*n);

    // could be generalized longterm:
    // - higher dimensions
    // - more than 4 points

    // find points with min/max x/y (/z/w/...)
    std::vector<int> min(d, 0);
    std::vector<int> max(d, 0);
    for(int i=0; i<n; ++i)
        for(int j=0; j<d; ++j)
        {
            if(interiorPoints[i][j] < interiorPoints[min[j]][j])
                min[j] = i;
            if(interiorPoints[i][j] > interiorPoints[max[j]][j])
                max[j] = i;
        }

    // for d=3 this needs to be a volume instead of a polygon
    // find and delete points inside the quadriliteral
    // http://totologic.blogspot.de/2014/01/accurate-point-in-triangle-test.html
    // do this by building a new list of vertices outside
    for(const Step<T>& i : interiorPoints)
        if(!pointInQuadrilateral(interiorPoints[min[0]], interiorPoints[max[1]], interiorPoints[max[0]], interiorPoints[min[1]], i))
            for(int j=0; j<d; ++j)
                coords.emplace_back(i[j]);

    // Also make sure that the min/max points are still considered
    for(int i=0; i<d; ++i)
    {
        for(int j=0; j<d; ++j)
            coords.emplace_back(interiorPoints[min[i]][j]);
        for(int j=0; j<d; ++j)
            coords.emplace_back(interiorPoints[max[i]][j]);
    }

    int k = coords.size()/d;
    LOG(LOG_TOO_MUCH) << "Akl Toussaint killed: "
            << (n - k) << "/" << n
            << " ("  << std::setprecision(2) << ((double) (n - k) / n * 100) << "%)";

    n = k;
}

template <class T>
void ConvexHull<T>::updateHullPoints() const
{
    orgQhull::QhullVertexList vl = qhull->vertexList();

    for(const auto &v : vl)
    {
        auto coord = v.point().coordinates();
        hullPoints_.emplace_back(std::vector<T>(coord, coord+d));
    }

    if(d==2) // for 2D we can order the points clockwise
    {
        // FIXME: will not work for the point at (0, 0)
        std::sort(hullPoints_.begin(), hullPoints_.end(),
            [](const Step<T> &a, const Step<T> &b) -> bool
            { return a.angle() < b.angle(); } );
    }
}

template <class T>
std::vector<std::vector<Step<T>>> ConvexHull<T>::hullFacets() const
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

template <>
inline int ConvexHull<double>::countZerosAndUpdateCmd()
{
    cmd = "QJ";
    return 0;
}

template <>
inline int ConvexHull<int>::countZerosAndUpdateCmd()
{
    // test, if points are fully dimensional
    // we need to do that first, since qhull seems to leak on exceptions
    int num_zeros = 0;
    int zero_axis = 0;
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

    if(num_zeros == 1)
    {
        LOG(LOG_DEBUG) << "Not full dimensional, strip one axis: " << zero_axis;
        // drop that dimension, see http://www.qhull.org/html/qh-optq.htm#Qb0
        cmd = "Qb"+std::to_string(zero_axis)+":0B"+std::to_string(zero_axis)+":0";
    }

    return num_zeros;
}


template <class T>
void ConvexHull<T>::runQhull()
{
    // test, if points are fully dimensional
    // we need to do that first, since qhull seems to leak on exceptions
    // TODO: replace by QJ for T==double
    int num_zeros = countZerosAndUpdateCmd();

    if(num_zeros >= 2)
    {
        LOG(LOG_DEBUG) << "Two dimensions less than fully dimensional";
        m_L = 0;
        m_A = 0;
        return;
    }
    else if(num_zeros == 1)
    {
        if(d - num_zeros <= 1)
        {
            LOG(LOG_DEBUG) << "One dimensional";
            m_L = 2*n;
            m_A = 0;
            return;
        }
        --d;
    }

    if(algorithm != CH_QHULL_AKL)
    {
        coords = std::vector<double>(n*d);
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
        LOG(LOG_ERROR) << e.what();
    }
}

template <class T>
void ConvexHull<T>::runAndrews()
{
    if(d != 2)
    {
        LOG(LOG_ERROR) << "Andrews Monotone Chain does only work in a plane (d=2), the data is d = " << d;
        throw std::invalid_argument("Andrews Monotone Chain does only work in a plane (d=2)");
    }

    int k = 0;
    // we need to sort the points, hence we make a copy
    if(algorithm != CH_ANDREWS_AKL)
    {
        pointSelection = std::vector<int>(n);
        for(int i=0; i<n; ++i)
            pointSelection[i] = i;
    }

    std::vector<int> hullPointMap(2*n);

    std::sort(pointSelection.begin(), pointSelection.end(),
        [&](const int a, const int b) -> bool
        {
            return interiorPoints[a] < interiorPoints[b];
        }
    );

    // Build lower hull
    for(int i=0; i<n; ++i)
    {
        while (k>=2 && cross2d_z(interiorPoints[hullPointMap[k-2]], interiorPoints[hullPointMap[k-1]], interiorPoints[pointSelection[i]]) <= 0)
            k--;
        hullPointMap[k++] = pointSelection[i];
    }

    // Build upper hull
    for(int i=n-2, t=k+1; i>=0; --i)
    {
        while (k>=t && cross2d_z(interiorPoints[hullPointMap[k-2]], interiorPoints[hullPointMap[k-1]], interiorPoints[pointSelection[i]]) <= 0)
            k--;
        hullPointMap[k++] = pointSelection[i];
    }

    hullPoints_ = std::vector<Step<T>>(k);
    for(int i=0; i<k; ++i)
        hullPoints_[i] = interiorPoints[hullPointMap[i]];

    // last point equals first, this makes calculation of A and L easier
}

template <class T>
void ConvexHull<T>::runJarvis()
{
    if(d > 3)
    {
        LOG(LOG_ERROR) << "Jarvis March does only work in d=2 and d=3, the data is d = " << d;
        throw std::invalid_argument("Jarvis March does only work in d=2 and d=3");
    }
    if(d != 2)
    {
        LOG(LOG_ERROR) << "Jarvis March is only implemented in d=2, the data is d = " << d;
        throw std::invalid_argument("Jarvis March is only implemented in d=2");
    }

    // Step < sorts by x value
    Step<T> p, p1;

    std::unordered_set<Step<T>> candidate_points;
    if(algorithm != CH_JARVIS_AKL)
    {
        candidate_points = std::unordered_set<Step<T>>(interiorPoints.begin(), interiorPoints.end());
        p1 = std::min(interiorPoints);
    }
    else
    {
        for(int i=0; i<n; ++i)
            candidate_points.insert(interiorPoints[pointSelection[i]]);
        p1 = std::min(interiorPoints);
    }

    candidate_points.erase(p1);
    hullPoints_.push_back(p1);

    int hull_idx = 0;
    do
    {
        // Search for a point 'p' such that orientation(hull.last, i, p) is
        // counterclockwise for all points 'i'
        p = p1;

        for(auto it = candidate_points.begin(); it != candidate_points.end(); ++it)
        {
            T orientation = cross2d_z(hullPoints_[hull_idx], *it, p);
            if(orientation > 0)
                p = *it;
            else if(orientation == 0) // colinear
            {
                // take the one furthest away
                if((p-hullPoints_[hull_idx]).length() < (*it-hullPoints_[hull_idx]).length())
                    p = *it;
                // and delete all other
            }
        }

        hull_idx++;
        hullPoints_.push_back(p);

        // found, then elimiminate all points inside the triangle (hull[0], hull[i-1], hull[i])
        candidate_points.erase(p);

        for(auto it = candidate_points.begin(); it != candidate_points.end(); )
        {
            if(pointInTriangle(hullPoints_[0],
                               hullPoints_[hull_idx-1],
                               hullPoints_[hull_idx],
                               *it))
                it = candidate_points.erase(it);
            else
                ++it;
        }
    } while(p!=p1); // if we reach the first, we have finished
    // mind that first and last entry of hullPoints_ are the same
}
