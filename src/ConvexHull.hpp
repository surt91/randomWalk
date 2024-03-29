#ifndef CONVEXHULL_H
#define CONVEXHULL_H

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

#include "Cmd.hpp"
#include "Logging.hpp"
#include "Step.hpp"
#include "geometry.hpp"

/** Class template for calculation of Convex Hulls.
 *
 * Uses an algorithm to calculate the convex
 * hull and some of its properties.
 *
 * All properties are lazy evaluated.
 *
 * \tparam T datatype of the Steps (integer or double)
 */
template <class T>
class ConvexHull
{
    protected:
        int n;
        int d;
        hull_algorithm_t algorithm;

        mutable double m_A;
        mutable double m_L;

        std::vector<Step<T>> *interiorPoints;
        mutable std::vector<Step<T>> hullPoints_;

    public:
        ConvexHull(hull_algorithm_t algorithm=CH_QHULL)
            : n(0),
              d(0),
              algorithm(algorithm)
        {
        }

        /// Constructs the hull of the given points.
        ConvexHull(std::vector<Step<T>> *interiorPoints, hull_algorithm_t algorithm)
            : n(0),
              d(0),
              algorithm(algorithm)
        {
            run(interiorPoints);
        }

        void run(std::vector<Step<T>> *interiorPoints);
        void setPoints(std::vector<Step<T>> *interiorPoints);
        void setHullAlgo(hull_algorithm_t alg);

        // observables
        double A() const;
        double L() const;
        int num_vertices() const;

        // hull
        const std::vector<Step<T>>& hullPoints() const;
        std::vector<std::vector<Step<T>>> hullFacets() const;

    protected:
        // for QHull
        void runQhull();
        void preprocessAklToussaintQHull();

        std::shared_ptr<orgQhull::Qhull> qhull;
        std::vector<double> coords;
        void updateHullPoints() const;
        int countZerosAndUpdateCmd(std::string &cmd);

        // for Andrews, and Jarvis
        void runAndrews();
        void runJarvis();
        void runChan();
        void preprocessAklToussaint();

        std::vector<Step<T>*> pointSelection;
        int zero_axis;
};

/// Calculates the convex hull of the given points.
template <class T>
void ConvexHull<T>::run(std::vector<Step<T>> *points)
{
    setPoints(points);

    switch(algorithm)
    {
        case CH_NOP:
            break;
        case CH_1D:
            break;
        case CH_QHULL_AKL:
            preprocessAklToussaintQHull();
            // fall through
        case CH_QHULL:
            runQhull();
            break;
        case CH_ANDREWS_AKL:
            preprocessAklToussaint();
            // fall through
        case CH_ANDREWS:
            runAndrews();
            break;
        case CH_JARVIS_AKL:
            preprocessAklToussaint();
            // fall through
        case CH_JARVIS:
            runJarvis();
            break;
        case CH_CHAN_AKL:
            preprocessAklToussaint();
            // fall through
        case CH_CHAN:
            runChan();
            break;
        default:
            LOG(LOG_ERROR) << "Algorithm not implemented, yet: "
                           << CH_LABEL[algorithm];
            throw std::invalid_argument("this is not implemented");
    }
}

/** Set the points and reset all observables.
 *
 * This is a preparation function, which will be called by run().
 */
template <class T>
void ConvexHull<T>::setPoints(std::vector<Step<T>> *points)
{
    interiorPoints = points;

    n = interiorPoints->size();
    d = (*interiorPoints)[0].d();

    // reset all observalbes, else the lazy evaluation will possibly
    // yield the values of the last call
    m_L = -1.;
    m_A = -1.;
    hullPoints_.clear();
}

/// Set the algorithm used for construction of the hull.
template <class T>
void ConvexHull<T>::setHullAlgo(hull_algorithm_t alg)
{
    algorithm = alg;
}

/// Volume (or in d=2 area) of the convex hull (lazy).
template <class T>
double ConvexHull<T>::A() const
{
    if(algorithm == CH_NOP)
        return 0;
    if(algorithm == CH_1D)
    {
        std::vector<Step<T>>& p = *interiorPoints;
        double min = p[0].x(), max = p[0].x();
        for(int i=0; i<n; ++i)
        {
                if(p[i].x() < min)
                    min = p[i].x();
                if(p[i].x() > max)
                    max = p[i].x();
        }
        return max - min;
    }

    if(m_A < 0)
    {
        if(d != 2)
        {
            LOG(LOG_ERROR) << "volume calculation only implemented for d=2";
            throw std::invalid_argument("volume calculation only implemented for d=2");
        }
        // calculate Area in 2d -- since the algorithm only works for d=2
        double a = 0;
        for(size_t i=0; i<hullPoints_.size()-1; ++i)
            a += (hullPoints_[i].x() - hullPoints_[i+1].x())
                * (hullPoints_[i].y() + hullPoints_[i+1].y());

        m_A = a/2;
    }

    return m_A;
}

/// Surface area (or in d=2 circumference) of the convex hull (lazy).
template <class T>
double ConvexHull<T>::L() const
{
    if(algorithm == CH_NOP)
        return 0;
    if(algorithm == CH_1D)
        return 2;

    if(m_L < 0)
    {
        if(d != 2)
        {
            LOG(LOG_ERROR) << "surface area calculation only implemented for d=2";
            throw std::invalid_argument("surface area calculation only implemented for d=2");
        }
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

/// number of vertices on the convex hull (lazy).
template <class T>
int ConvexHull<T>::num_vertices() const
{
    // first and last are listed twice
    int c = hullPoints().size() - 1;

    // qhull does not list first and last twice
    if(algorithm == CH_QHULL_AKL || algorithm == CH_QHULL)
        c++;

    return c;
}

/// Vertices of the convex hull (lazy).
template <class T>
const std::vector<Step<T>>& ConvexHull<T>::hullPoints() const
{
    if(algorithm == CH_NOP || algorithm == CH_1D)
        return hullPoints_;
    if(hullPoints_.empty())
        updateHullPoints();
    LOG(LOG_TOO_MUCH) << "Convex Hull: " << hullPoints_;
    return hullPoints_;
}

/// deletes points from interior points according to the Akl Toussaint heuristic
/// \image html akl.svg "discarded points for 4 and 8 points"
template <class T>
void ConvexHull<T>::preprocessAklToussaint()
{
    // could be generalized longterm:
    // - higher dimensions
    // - more than 4 points
    pointSelection.clear();
    std::vector<Step<T>>& p = *interiorPoints;

    // find points with min/max x/y (/z/w/...)
    std::vector<Step<T>*> min(d, &p[0]);
    std::vector<Step<T>*> max(d, &p[0]);
    // also some bonus points: min x+y, min x-y, max x+y, max x-y
    Step<T>* minxpy = &p[0];
    Step<T>* maxxpy = &p[0];
    Step<T>* minxmy = &p[0];
    Step<T>* maxxmy = &p[0];
    // TODO: generalize to arbitrary dimensions
    for(int i=0; i<n; ++i)
    {
        for(int j=0; j<d; ++j)
        {
            if(p[i][j] < min[j]->x(j))
                min[j] = &p[i];
            if(p[i][j] > max[j]->x(j))
                max[j] = &p[i];
        }
        if(p[i][0] + p[i][1] < minxpy->x(0) + minxpy->x(1))
            minxpy = &p[i];
        if(p[i][0] + p[i][1] > maxxpy->x(0) + maxxpy->x(1))
            maxxpy = &p[i];
        if(p[i][0] - p[i][1] < minxmy->x(0) - minxmy->x(1))
            minxmy = &p[i];
        if(p[i][0] - p[i][1] > maxxmy->x(0) - maxxmy->x(1))
            maxxmy = &p[i];
    }

    // for d=3 this needs to be a volume instead of a polygon
    // find and delete points inside the quadriliteral
    // http://totologic.blogspot.de/2014/01/accurate-point-in-triangle-test.html
    // do this by building a new list of vertices outside
    for(int i=0; i<n; ++i)
        if(!pointInOcto(*min[0],
                        *minxmy,
                        *max[1],
                        *maxxpy,
                        *max[0],
                        *maxxmy,
                        *min[1],
                        *minxpy,
                        p[i]))
            pointSelection.emplace_back(&p[i]);

    // Also make sure that the min/max points are still considered
    for(int i=0; i<d; ++i)
    {
        pointSelection.emplace_back(min[i]);
        pointSelection.emplace_back(max[i]);
    }

    // ensure that we do not add the same point twice
    if(maxxmy != max[0] && maxxmy != min[1])
        pointSelection.emplace_back(maxxmy);
    if(maxxpy != max[0] && maxxpy != max[1])
        pointSelection.emplace_back(maxxpy);
    if(minxpy != min[0] && minxpy != min[1])
        pointSelection.emplace_back(minxpy);
    if(minxmy != min[0] && minxmy != max[1])
        pointSelection.emplace_back(minxmy);

    LOG(LOG_TOO_MUCH) << "Akl Toussaint killed: "
            << (n - pointSelection.size()) << "/" << n
            << " ("  << std::setprecision(2)
            << ((double) (n - pointSelection.size()) / n * 100) << "%)";

    n = pointSelection.size();
}

/// deletes points from interior points according to the Akl Toussaint heuristic
/// \image html akl.svg "discarded points for 4 and 8 points"
template <class T>
void ConvexHull<T>::preprocessAklToussaintQHull()
{
    if(d != 2 && d != 3)
    {
        LOG(LOG_ERROR) << "Akl heuristic only implemented for d=2 and d=3";
        throw std::invalid_argument("Akl heuristic only implemented for d=2 and d=3");
    }

    // could be generalized longterm:
    // - higher dimensions
    // - more than 4 points

    coords.clear();
    std::vector<Step<T>>& p = *interiorPoints;

    // find points with min/max x/y (/z/w/...)
    std::vector<Step<T>*> min(d, &p[0]);
    std::vector<Step<T>*> max(d, &p[0]);

    for(int i=0; i<n; ++i)
    {
        for(int j=0; j<d; ++j)
        {
            if(p[i][j] < min[j]->x(j))
                min[j] = &p[i];
            if(p[i][j] > max[j]->x(j))
                max[j] = &p[i];
        }
    }

    // Also make sure that the min/max points are still considered
    for(int i=0; i<d; ++i)
    {
        for(int j=0; j<d; ++j)
            coords.emplace_back(min[i]->x(j));
        for(int j=0; j<d; ++j)
            coords.emplace_back(max[i]->x(j));
    }

    if(d == 2)
    {
        // also some bonus points: min x+y, min x-y, max x+y, max x-y
        Step<T>* minxpy = &p[0];
        Step<T>* maxxpy = &p[0];
        Step<T>* minxmy = &p[0];
        Step<T>* maxxmy = &p[0];
        // TODO: generalize to arbitrary dimensions
        for(int i=0; i<n; ++i)
        {
            if(p[i][0] + p[i][1] < minxpy->x(0) + minxpy->x(1))
                minxpy = &p[i];
            if(p[i][0] + p[i][1] > maxxpy->x(0) + maxxpy->x(1))
                maxxpy = &p[i];
            if(p[i][0] - p[i][1] < minxmy->x(0) - minxmy->x(1))
                minxmy = &p[i];
            if(p[i][0] - p[i][1] > maxxmy->x(0) - maxxmy->x(1))
                maxxmy = &p[i];
        }

        // for d=3 this needs to be a volume instead of a polygon
        // find and delete points inside the quadriliteral
        // http://totologic.blogspot.de/2014/01/accurate-point-in-triangle-test.html
        // do this by building a new list of vertices outside
        for(int i=0; i<n; ++i)
            if(!pointInOcto(*min[0],
                            *minxmy,
                            *max[1],
                            *maxxpy,
                            *max[0],
                            *maxxmy,
                            *min[1],
                            *minxpy,
                            p[i]))
                for(int j=0; j<d; ++j)
                    coords.emplace_back(p[i][j]);

        // ensure that we do not add the same point twice
        if(maxxmy != max[0] && maxxmy != min[1])
            for(int j=0; j<d; ++j)
                coords.emplace_back(maxxmy->x(j));
        if(maxxpy != max[0] && maxxpy != max[1])
            for(int j=0; j<d; ++j)
                coords.emplace_back(maxxpy->x(j));
        if(minxpy != min[0] && minxpy != min[1])
            for(int j=0; j<d; ++j)
                coords.emplace_back(minxpy->x(j));
        if(minxmy != min[0] && minxmy != max[1])
            for(int j=0; j<d; ++j)
                coords.emplace_back(minxmy->x(j));
    }
    if(d == 3)
    {
        const int numAddPoints = 8;
        std::vector<Step<T>*> sumPoints(numAddPoints, &p[0]);
        std::vector<T> sums(numAddPoints, 0);
        int signs[][3] =
            {
                { 1, 1, 1},
                { 1, 1,-1},
                { 1,-1, 1},
                {-1, 1, 1},
                { 1,-1,-1},
                {-1, 1,-1},
                {-1,-1, 1},
                {-1,-1,-1},
                //~ { 1, 1, 0},
                //~ { 1,-1, 0},
                //~ {-1,-1, 0},
                //~ {-1, 1, 0},
                //~ { 1, 0, 1},
                //~ { 1, 0,-1},
                //~ {-1, 0, 1},
                //~ {-1, 0,-1},
                //~ { 0, 1, 1},
                //~ { 0, 1,-1},
                //~ { 0,-1, 1},
                //~ { 0,-1,-1}
            };

        for(int k=0; k<numAddPoints; ++k)
            for(int i=0; i<n; ++i)
            {
                auto tmp = p[i][0]*signs[k][0]
                         + p[i][1]*signs[k][1]
                         + p[i][2]*signs[k][2];
                if(tmp > sums[k])
                {
                    sumPoints[k] = &p[i];
                    sums[k] = tmp;
                }
            }

        for(int k=0; k<numAddPoints; ++k)
            for(int j=0; j<d; ++j)
                coords.emplace_back(sumPoints[k]->x(j));

        std::string cmd;
        auto zeros = countZerosAndUpdateCmd(cmd);
        if(zeros >= 2)
            return;

        // replace by something less overkill
        // a custom gift wrapping implementation?
        auto q = std::make_shared<orgQhull::Qhull>("", d, (int) coords.size()/d,
                                                   coords.data(), cmd.c_str());

        for(int i=0; i<n; ++i)
            if(!pointInFacets(q->facetList(), p[i]))
                for(int j=0; j<d; ++j)
                    coords.emplace_back(p[i][j]);
    }

    int k = coords.size()/d;
    LOG(LOG_DEBUG) << "Akl Toussaint killed: "
            << (n - k) << "/" << n
            << " ("  << std::setprecision(2)
            << ((double) (n - k) / n * 100) << "%)";

    n = k;
}

template <class T>
void ConvexHull<T>::updateHullPoints() const
{
    orgQhull::QhullVertexList vl = qhull->vertexList();
    hullPoints_.clear();

    for(const auto &v : vl)
    {
        auto coord = v.point().coordinates();
        // for d>=3, if the points are in a hyperplane, coord will have
        // (d-1)*(n+1) entries instead of d*(n+1)
        // This leads to a invalid read
        // It must be determined which dimension was dropped and zeros
        // need to be written into hullPoints_
        if(zero_axis >= 0 && v.point().dimension() != d)
        {
            std::vector<T> vec;
            vec.reserve(d);
            for(int i=0; i<d-1; ++i)
            {
                if(zero_axis == i)
                    vec.push_back(0);
                vec.push_back(coord[i]);
            }
            hullPoints_.emplace_back(vec);
        }
        else
        {
            hullPoints_.emplace_back(std::vector<T>(coord, coord+d));
        }
    }

    if(d==2) // for 2D we can order the points clockwise
    {
        // determine interior point
        Step<T> p(d);
        for(auto &point : hullPoints_)
            p += point;
        p /= hullPoints_.size();

        std::sort(hullPoints_.begin(), hullPoints_.end(),
            [&p](const Step<T> &a, const Step<T> &b) -> bool
            { return (p-a).angle() < (p-b).angle(); } );
    }
}

template <class T>
std::vector<std::vector<Step<T>>> ConvexHull<T>::hullFacets() const
{
    if(d < 3)
    {
        LOG(LOG_ERROR) << "facets only implemented for d >= 3";
        throw std::invalid_argument("facets only implemented for d >= 3");
    }
    if(d > 3)
    {
        LOG(LOG_ERROR) << "facets not well thought through for d>=4";
        throw std::invalid_argument("facets not well thought through for d>=4");
    }

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
            for(size_t k=0; k<=facet.size() - d; k+=d)
            {
                std::vector<Step<T>> simplex(facet.begin()+k, facet.begin()+k+d);
                LOG(LOG_TOO_MUCH) << simplex;
                facet.push_back(facet[k]);
                facet.push_back(facet[k+d-1]); // not sure for d>3
                facets.push_back(simplex);
            }
        }
        else
            facets.push_back(facet);
    }
    return facets;
}

template <>
inline int ConvexHull<double>::countZerosAndUpdateCmd(std::string &cmd)
{
    zero_axis = -1;
    cmd = "Qt";
    if(Logger::verbosity <= LOG_INFO)
        cmd += " Pp";
    return 0;
}

template <>
inline int ConvexHull<int>::countZerosAndUpdateCmd(std::string &cmd)
{
    // test, if points are fully dimensional
    // we need to do that first, since qhull seems to leak on exceptions
    int num_zeros = 0;
    zero_axis = -1;
    int limit = coords.size() / d;
    for(int i=0; i<d; ++i)
    {
        int j = 0;
        // test if one column contains only one value -> no extesnsion in this
        // dimension
        while(j < limit && coords[j*d+i] == coords[i])
            ++j;
        if(j == limit)
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

    if(Logger::verbosity <= LOG_INFO)
        cmd += " Pp";

    return num_zeros;
}


template <class T>
void ConvexHull<T>::runQhull()
{
    // test, if points are fully dimensional
    // we need to do that first, since qhull seems to leak on exceptions

    if(algorithm != CH_QHULL_AKL)
    {
        if(coords.size() != (size_t) n*d)
            coords = std::vector<double>(n*d);

        for(int i=0; i<n; ++i)
            for(int j=0; j<d; ++j)
                coords[i*d + j] = (*interiorPoints)[i][j];
    }

    std::string cmd;
    int num_zeros = countZerosAndUpdateCmd(cmd);

    if(num_zeros >= 2)
    {
        LOG(LOG_DEBUG) << "Two dimensions less than fully dimensional";
        m_L = 0;
        m_A = 0;
        return;
    }
    else if(num_zeros == 1)
    {
        if(d == 2)
        {
            LOG(LOG_DEBUG) << "One dimensional";
            // since its two d, this will switch 0 and 1
            int nonzero_axis = !zero_axis;
            double span = 0;
            double mini = 0, maxi = 0 ;
            for(const auto &s : *interiorPoints)
            {
                if(s[nonzero_axis] < mini)
                    mini = s[nonzero_axis];
                if(s[nonzero_axis] > maxi)
                    maxi = s[nonzero_axis];
            }
            span = maxi-mini;

            m_L = 2*span;
            m_A = 0;
            return;
        }
    }

    try
    {
        // limitation of Qhull (QH6271), just supress it,
        // should not do any harm occurs seldom for d >= 4, see also:
        // http://www.qhull.org/html/qh-impre.htm#limit
        if(d >= 4)
            cmd += " Q12";

        // comment, dimension, count, coordinates[], command
        qhull = std::make_shared<orgQhull::Qhull>("", d, n, coords.data(), cmd.c_str());
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
        LOG(LOG_ERROR) << "#discarded axes: " << num_zeros;
        LOG(LOG_ERROR) << "coords: " << coords;
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
        pointSelection = std::vector<Step<T>*>(n);
        for(int i=0; i<n; ++i)
            pointSelection[i] = &(*interiorPoints)[i];
    }

    std::vector<Step<T>*> hullPointMap(2*n);

    std::sort(pointSelection.begin(), pointSelection.end(),
        [](const Step<T> *a, const Step<T> *b) -> bool
        {
            return *a < *b;
        }
    );

    // Build lower hull
    for(int i=0; i<n; ++i)
    {
        while (k>=2 && cross2d_z(*hullPointMap[k-2], *hullPointMap[k-1], *pointSelection[i]) <= 0)
            k--;
        hullPointMap[k++] = pointSelection[i];
    }

    // Build upper hull
    for(int i=n-2, t=k+1; i>=0; --i)
    {
        while (k>=t && cross2d_z(*hullPointMap[k-2], *hullPointMap[k-1], *pointSelection[i]) <= 0)
            k--;
        hullPointMap[k++] = pointSelection[i];
    }

    hullPoints_ = std::vector<Step<T>>(k);
    for(int i=0; i<k; ++i)
        hullPoints_[i] = *hullPointMap[i];

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
        candidate_points = std::unordered_set<Step<T>>(interiorPoints->begin(), interiorPoints->end());
        p1 = std::min(*interiorPoints);
    }
    else
    {
        for(int i=0; i<n; ++i)
            candidate_points.insert(*pointSelection[i]);
        p1 = std::min(*interiorPoints);
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

// this is the same andrews implementation as above, but as a pure function
// TODO join this with the above implementation
template <class T>
std::vector<Step<T>> andrew(std::vector<Step<T>> &points)
{
    // sort points lexicographically
    std::sort(points.begin(), points.end());

    int n = points.size();
    std::vector<Step<T>> hull(2*n);

    int k = 0;
    // Build lower hull
    for(int i=0; i<n; ++i)
    {
        while (k>=2 && cross2d_z(hull[k-2], hull[k-1], points[i]) <= 0)
            k--;
        hull[k++] = points[i];
    }

    // Build upper hull
    for(int i=n-2, t=k+1; i>=0; --i)
    {
        while (k>=t && cross2d_z(hull[k-2], hull[k-1], points[i]) <= 0)
            k--;
        hull[k++] = points[i];
    }

    hull.resize(k);
    return hull;
}

template <class T>
void ConvexHull<T>::runChan()
{
    if(d > 3)
    {
        LOG(LOG_ERROR) << "Chan's Algorithm does only work in d=2 and d=3, the data is d = " << d;
        throw std::invalid_argument("Chan's Algorithm does only work in d=2 and d=3");
    }
    if(d != 2)
    {
        LOG(LOG_ERROR) << "Chan's Algorithm is only implemented in d=2, the data is d = " << d;
        throw std::invalid_argument("Chan's Algorithm is only implemented in d=2");
    }

    int n = interiorPoints->size();
    int m = 100;
    std::vector<Step<T>> hull;
    while(true)
    {
        int k = n/m + 1;
        // split point set in n/m sub sets of size <= m
        auto subsets = std::vector<std::vector<Step<T>>>(k);
        for(int i=0; i<n; ++i)
            subsets[i%k].push_back((*interiorPoints)[i]);

        // calculate the sub-hulls with Andrew's algorithm
        // the implementation above is f***ed up, just reimplement it :/
        auto subhulls = std::vector<std::vector<Step<T>>>(k);
        for(int i=0; i<k; ++i)
            subhulls[i] = andrew(subsets[i]);

        // perform Jarvis' march with only the tangent points
        // find p1: leftmost point
        Step<T> p1 = std::min(*interiorPoints);
        Step<T> q = p1;
        hull.push_back(p1);
        for(int i=0; i<m; ++i)
        {
            for(auto j=0; j<k; ++j)
            {
                Step<T> t = polygon_tangent(hull[i], subhulls[j]);

                T orientation = cross2d_z(hull[i], t, q);
                if(orientation > 0)
                    q = t;
                else if(orientation == 0) // colinear
                {
                    // take the one furthest away
                    if((q-hull[i]).length() < (t-hull[i]).length())
                        q = t;
                }
            }
            hull.push_back(q);

            // if we are finished after <= m iterations, we found the hull
            if(q == p1)
            {
                hullPoints_ = hull;
                return;
            }
        }

        m = m*m;
        hull.clear();
    }
}

#endif
