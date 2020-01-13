#ifndef SPECWALKER_H
#define SPECWALKER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>

#include "../io.hpp"
#include "../Cmd.hpp"
#include "../visualization/Svg.hpp"
#include "../visualization/Povray.hpp"
#include "../visualization/Gnuplot2D.hpp"
#include "../visualization/Gnuplot3D.hpp"
#include "../visualization/Threejs.hpp"
#include "../RNG.hpp"
#include "../Step.hpp"
#include "../ConvexHull.hpp"
#include "Walker.hpp"

/// Signum function
/// returns -1, 0 or 1
/// http://stackoverflow.com/a/4609795
template <typename T> int sign(T val)
{
    return (T(0) < val) - (val < T(0));
}

/** Abstract Class Template SpecWalker.
 *
 * Implements generic things offered by the Walker interface.
 * Is Base Class of all Walker Implementations.
 *
 * \tparam T datatype of the coordinates of Step (int or double)
 */
template <class T>
class SpecWalker : public Walker
{
    public:
        SpecWalker(int d, int numSteps, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false)
            : Walker(d, numSteps, rng, hull_algo, amnesia),
              m_points(numSteps+1, Step<T>(d))
        {
        }

        SpecWalker(const SpecWalker &) = default;
        virtual ~SpecWalker() = default;

        void init();
        virtual void reconstruct() override;
        virtual void generate_independent_sample() override;

        const ConvexHull<T>& convexHull() const;
        virtual void setHullAlgo(hull_algorithm_t a) override final;

        /// function to make the type of T accessable outside (e.g. per decltype)
        static T T_type() { return T(); }

        ///\name observables
        double A() const final { return convexHull().A(); }
        double L() const final { return convexHull().L(); }
        double maxDiameter() const final;
        double r() const final;
        double r2() const final;
        double rx() const final;
        double ry() const final;
        virtual double argminx() const override;
        virtual double argmaxx() const override;
        double minx() const final;
        double maxx() const final;
        int num_on_hull() const final;
        double oblateness() const final;
        double length() const final;
        int steps_taken() const final;
        virtual int num_resets() const override;
        virtual int maxsteps_partialwalk() const override;
        int visitedSites() const final;
        int enclosedSites() const final;
        int passage(int t1=0, int axis=0) const final;
        std::vector<double> correlation(std::vector<int> t, int axis=0) const final;

        ///\name get state
        const std::vector<Step<T>>& steps() const { return m_steps; }
        const std::vector<Step<T>>& points() const { return m_points; }
        const std::vector<Step<T>>& hullPoints() const { return m_convex_hull.hullPoints(); }

        ///\name update state
        virtual void updateSteps() override = 0;
        virtual void updatePoints(int start=1) override;
        virtual void updateHull() override;

        ///\name visualization
        virtual void svg(const std::string filename, const bool with_hull=false) const override;
        virtual void svgOfChange(std::string filename, UniformRNG &rng_in) override;
        virtual void pov(const std::string filename, const bool with_hull=false) const override;
        virtual void gp(const std::string filename, const bool with_hull=false) const override;
        virtual void threejs(const std::string filename, const bool with_hull=false) const override;
        std::string print() const final;

        ///\name degenerate cases
        virtual void degenerateMaxVolume() override;
        virtual void degenerateMaxSurface() override;
        virtual void degenerateMinVolume() override;
        virtual void degenerateMinSurface() override;

        void goDownhill(const bool maximize, const wanted_observable_t observable, const int stagnate=1000) final;

    protected:
        std::vector<Step<T>> m_steps;
        std::vector<Step<T>> m_points;
        ConvexHull<T> m_convex_hull;
        ConvexHull<T> m_old_convex_hull;
};

/// Do initialization, e.g. calculate the steps and the hull.
template <class T>
void SpecWalker<T>::init()
{
    updateSteps();
    updatePoints();
    setHullAlgo(hull_algo); // does also update hull
}

/// Get new random numbers and reconstruct the walk
template <class T>
void SpecWalker<T>::reconstruct()
{
    // write new random numers into our state
    std::generate(random_numbers.begin(), random_numbers.end(), std::ref(rng));
    init();
}

/// For most Walks this will be just a simple reconstruct, but
/// e.g., SAW will need a MCMC equilibration with the pivot algorithm
/// which will be performed in this call
template <class T>
void SpecWalker<T>::generate_independent_sample()
{
    reconstruct();
}

/// Update the convex hull given the current points.
template <class T>
void SpecWalker<T>::updateHull()
{
    m_convex_hull.run(&m_points);
}

template <class T>
const ConvexHull<T>& SpecWalker<T>::convexHull() const
{
    return m_convex_hull;
}

/// Changes the algorithm used to calculate the hull.
template <class T>
void SpecWalker<T>::setHullAlgo(const hull_algorithm_t a)
{
    m_convex_hull.setHullAlgo(a);
    updateHull();
    hull_algo = a;
}

/// Updates the points of the walk given its steps.
template <class T>
void SpecWalker<T>::updatePoints(const int start)
{
    for(int i=start; i<=numSteps; ++i)
    {
        m_points[i].setZero();
        m_points[i] += m_points[i-1];
        m_points[i] += m_steps[i-1];
    }
}

/** Save a gnuplot file visualizing the walk.
 *
 * Works only in d=2. Otherwise yields a projection to d=2.
 *
 * \param filename Basename of the outputfile.
 * \param with_hull Visualize only the walk or also its hull.
 */
template <class T>
void SpecWalker<T>::svg(const std::string filename, const bool with_hull) const
{
    SVG pic(filename);
    const std::vector<Step<T>> p = points();
    std::vector<std::vector<double>> points;
    int min_x=0, max_x=0, min_y=0, max_y=0;
    for(auto i : p)
    {
        T x1 = i[0], y1 = i[1];
        std::vector<double> point {(double) x1, (double) y1};

        pic.circle(x1, y1, true);

        points.push_back(point);

        if(x1 < min_x)
            min_x = x1;
        if(x1 > max_x)
            max_x = x1;
        if(y1 < min_y)
            min_y = y1;
        if(y1 > max_y)
            max_y = y1;
    }
    pic.polyline(points);

    if(d > 2)
        pic.text(min_x, max_y-20, "projected from d=" + std::to_string(d), "red");

    points.clear();
    if(with_hull)
    {
        const std::vector<Step<T>> h = hullPoints();
        for(auto &i : h)
        {
            std::vector<double> point {(double) i[0], (double) i[1]};
            points.push_back(point);
        }
        pic.polyline(points, true, std::string("red"));
    }
    pic.setGeometry(min_x -1, min_y - 1, max_x + 1, max_y + 1);
    pic.save();
}


template <class T>
void SpecWalker<T>::svgOfChange(std::string filename, UniformRNG &rng)
{
    SVG pic(filename);
    std::vector<Step<T>> p = points();
    std::vector<std::vector<double>> ps;
    int min_x=0, max_x=0, min_y=0, max_y=0;
    for(auto i : p)
    {
        T x1 = i[0], y1 = i[1];
        std::vector<double> point {(double) x1, (double) y1};

        pic.circle(x1, y1, true, "grey");

        ps.push_back(point);

        if(x1 < min_x)
            min_x = x1;
        if(x1 > max_x)
            max_x = x1;
        if(y1 < min_y)
            min_y = y1;
        if(y1 > max_y)
            max_y = y1;
    }
    pic.polyline(ps, false, "grey");

    change(rng);
    p = points();

    ps.clear();
    for(auto i : p)
    {
        T x1 = i[0], y1 = i[1];
        std::vector<double> point {(double) x1, (double) y1};

        pic.circle(x1, y1, true);

        ps.push_back(point);

        if(x1 < min_x)
            min_x = x1;
        if(x1 > max_x)
            max_x = x1;
        if(y1 < min_y)
            min_y = y1;
        if(y1 > max_y)
            max_y = y1;
    }
    pic.polyline(ps, false);

    if(d > 2)
        pic.text(min_x, max_y-20, "projected from d=" + std::to_string(d), "red");

    pic.setGeometry(min_x -1, min_y - 1, max_x + 1, max_y + 1);
    pic.save();
}


/** Save a povray file visualizing the walk.
 *
 * Works only in d=3.
 *
 * \param filename Basename of the outputfile.
 * \param with_hull Visualize only the walk or also its hull.
 */
template <class T>
void SpecWalker<T>::pov(const std::string filename, const bool with_hull) const
{
    Povray pic(filename);
    const std::vector<Step<T>> p = points();
    std::vector<std::vector<double>> points;
    for(auto i : p)
    {
        T x = i[0], y = i[1], z = 0;
        if(d > 2)
            z = i[2];
        std::vector<double> point {(double) x, (double) y, (double) z};

        points.push_back(point);
    }
    pic.polyline(points);

    points.clear();
    if(with_hull && d > 2)
    {
        const std::vector<std::vector<Step<T>>> h = convexHull().hullFacets();
        for(auto &i : h)
        {
            std::vector<double> p1 {(double) i[0][0], (double) i[0][1], (double) i[0][2]};
            std::vector<double> p2 {(double) i[1][0], (double) i[1][1], (double) i[1][2]};
            std::vector<double> p3 {(double) i[2][0], (double) i[2][1], (double) i[2][2]};
            pic.facet(p1, p2, p3);
        }
    }

    pic.save();
}

/** Save a threejs file visualizing the walk.
 *
 * Works only in d=2 and d=3.
 *
 * \param filename Basename of the outputfile.
 * \param with_hull Visualize only the walk or also its hull.
 */
template <class T>
void SpecWalker<T>::threejs(const std::string filename, const bool with_hull) const
{
    Threejs pic(filename);
    const std::vector<Step<T>> p = points();
    std::vector<std::vector<double>> points;
    for(auto i : p)
    {
        T x = i[0], y = i[1], z = 0;
        if(d > 2)
            z = i[2];
        std::vector<double> point {(double) x, (double) y, (double) z};

        points.push_back(point);
    }
    pic.polyline(points);

    points.clear();
    if(with_hull && d > 2)
    {
        const std::vector<std::vector<Step<T>>> h = convexHull().hullFacets();

        double mx = 0, my = 0, mz = 0;
        auto &hullPoints = convexHull().hullPoints();
        for(auto &i : hullPoints)
        {
            mx += i[0];
            my += i[1];
            mz += i[2];
        }
        mx /= hullPoints.size();
        my /= hullPoints.size();
        mz /= hullPoints.size();
        const Step<T> origin(std::vector<T>({(T) mx, (T) my, (T) mz}));


        for(auto &i : h)
        {
            std::vector<double> p1 {(double) i[0][0], (double) i[0][1], (double) i[0][2]};
            std::vector<double> p2 {(double) i[1][0], (double) i[1][1], (double) i[1][2]};
            std::vector<double> p3 {(double) i[2][0], (double) i[2][1], (double) i[2][2]};

            // add both, since our hull should be transparent,
            // we want to see them from inside
            //~ pic.facet(p1, p3, p2);
            //~ pic.facet(p1, p2, p3);

            if(side(i[0], i[1], i[2], origin) < 0)
            {
                // the origin is in front of the facet -> reorder for good normals
                pic.facet(p1, p3, p2);
            }
            else
            {
                // the origin is behind the facet -> everything is good
                pic.facet(p1, p2, p3);
            }
        }
    }

    pic.save();
}

/** Save a gnuplot file visualizing the walk.
 *
 * Works only in d=2 and d=3.
 *
 * \param filename Basename of the outputfile.
 * \param with_hull Visualize only the walk or also its hull.
 */
template <class T>
void SpecWalker<T>::gp(const std::string filename, const bool with_hull) const
{
    if(d==2)
    {
        Gnuplot2D pic(filename);

        const std::vector<Step<T>> p = points();
        std::vector<std::vector<double>> points;
        for(auto i : p)
        {
            T x = i[0], y = i[1];
            std::vector<double> point {(double) x, (double) y};

            points.push_back(point);
        }
        pic.polyline(points);

        points.clear();
        if(with_hull)
        {
            const std::vector<Step<T>> h = hullPoints();
            for(auto &i : h)
            {
                std::vector<double> point {(double) i[0], (double) i[1]};
                points.push_back(point);
            }
            pic.polyline(points, true);
        }

        pic.save();
    }
    else if(d==3)
    {
        Gnuplot3D pic(filename);
        const std::vector<Step<T>> p = points();
        std::vector<std::vector<double>> points;
        for(auto i : p)
        {
            T x = i[0], y = i[1], z = i[2];
            std::vector<double> point {(double) x, (double) y, (double) z};

            points.push_back(point);
        }
        pic.polyline(points);

        points.clear();
        if(with_hull && d > 2)
        {
            const std::vector<std::vector<Step<T>>> h = convexHull().hullFacets();
            for(auto &i : h)
            {
                std::vector<double> p1 {(double) i[0][0], (double) i[0][1], (double) i[0][2]};
                std::vector<double> p2 {(double) i[1][0], (double) i[1][1], (double) i[1][2]};
                std::vector<double> p3 {(double) i[2][0], (double) i[2][1], (double) i[2][2]};
                pic.facet(p1, p2, p3);
            }
        }

        pic.save();
    }
    else
    {
        LOG(LOG_WARNING) << "Gnuplot output only implemented for d = 2 and d = 3";
    }
}

/// Get a human readable representation of the walk.
template <class T>
std::string SpecWalker<T>::print() const
{
    std::stringstream ss;
    for(auto i : points())
        ss << i << " ";
    ss << "\n";
    return ss.str();
}

/// Get the maximum distance of any two points of the walk.
template <class T>
double SpecWalker<T>::maxDiameter() const
{
    double maxD = 0;
    int n_hullpoints = hullPoints().size();
    for(int i=0; i<n_hullpoints; ++i)
        for(int j=0; j<i; ++j)
        {
            double diameter = (hullPoints()[i] - hullPoints()[j]).length();
            if(diameter > maxD)
                maxD = diameter;
        }

    return maxD;
}

/// Get the oblateness of the walk
/// (this is not really oblateness but more the ratio of the longest and shortest diameter)
template <class T>
double SpecWalker<T>::oblateness() const
{
    double maxLongAxis = 0.;
    int maxI = 0;
    int maxJ = 0;
    int n_hullpoints = hullPoints().size();

    // first: search the two points with largest distance
    for(int i=0; i<n_hullpoints; ++i)
        for(int j=0; j<i; ++j)
        {
            double diameter = (hullPoints()[i] - hullPoints()[j]).length();
            if(diameter > maxLongAxis)
            {
                maxLongAxis = diameter;
                maxI = i;
                maxJ = j;
            }
        }

    double maxShortAxis = 0.;
    double maxCross = 0.;
    // second search point with largest distance to the axis between i and j
    // that is the point p which (in d = 2)
    // maximizes |cross2d(i, p, j)|
    for(int k=0; k<n_hullpoints; ++k)
    {
        double cross = std::abs(cross2d_z(hullPoints()[maxI], hullPoints()[k], hullPoints()[maxJ]));
        if(cross > maxCross)
        {
            maxShortAxis = cross / hullPoints()[maxI].dist(hullPoints()[maxJ]);
            maxCross = cross;
        }
    }

    // divide by 2 maxShortAxis, to give 1 for a sphere
    // will result in unintuitive values for asymmetric shapes
    return maxLongAxis/maxShortAxis/2.;
}

/// Get the length of the walk
template <class T>
double SpecWalker<T>::length() const
{
    double len = 0.;
    for(auto s : steps())
        len += s.length();
    return len;
}

/// Get the lnumber of steps taken by the walk (mainly useful for RTP, fixed t)
template <class T>
int SpecWalker<T>::steps_taken() const
{
    return numSteps;
}

template <class T>
int SpecWalker<T>::num_resets() const
{
    return 0;
}

template <class T>
int SpecWalker<T>::maxsteps_partialwalk() const
{
    return steps_taken();
}

template <>
inline int SpecWalker<double>::visitedSites() const
{
    return -1;
}

/// Get the number of distinct visited sites
template <>
inline int SpecWalker<int>::visitedSites() const
{
    std::set<Step<int>> distinctPoints;
    for(auto &p : points())
        distinctPoints.insert(p);
    return distinctPoints.size();
}

template <>
inline int SpecWalker<double>::enclosedSites() const
{
    return -1;
}

/// Get the number of distinct visited sites
template <>
inline int SpecWalker<int>::enclosedSites() const
{
    // derive a bounding box from the convex hull,
    // extend it by 1 in every direction
    // perform a DFS (visited sites can not be entered)
    // every not-encountered site is enclosed by the walk
    std::unordered_set<Step<int>> distinctPoints;
    std::unordered_set<Step<int>> encountered_sites;
    std::vector<Step<int>> stack;


    std::vector<int> max(d, 0), min(d, 0);
    for(auto &p : points())
    {
        for(int i=0; i<d; ++i)
        {
            if(p[i] > max[i])
                max[i] = p[i];
            if(p[i] < min[i])
                min[i] = p[i];
        }
        distinctPoints.insert(p);
    }

    for(int i=0; i<d; ++i)
    {
        ++max[i];
        --min[i];
    }

    int size = 1;
    for(int i=0; i<d; ++i)
    {
        size *= max[i] - min[i] + 1;
    }

    // start in a corner
    Step<int> start(max);
    stack.push_back(start);
    encountered_sites.insert(start);
    while(!stack.empty())
    {
        Step<int> current = stack.back();
        stack.pop_back();

        for(auto &n : current.neighbors())
        {
            bool inside_bounds = true;
            for(int i=0; i<d; ++i)
                if(n[i] > max[i] || n[i] < min[i])
                    inside_bounds = false;

            if(inside_bounds && !encountered_sites.count(n) && !distinctPoints.count(n))
            {
                stack.push_back(n);
                encountered_sites.insert(n);
            }
        }
    }

    return size - encountered_sites.size();
}

/// Get the end-to-end distance of the walk.
template <class T>
double SpecWalker<T>::r() const
{
    return (points().front() - points().back()).length();
}

/// Get the x-component of the end-to-end distance of the walk.
template <class T>
double SpecWalker<T>::rx() const
{
    return (points().front() - points().back()).x();
}

/// Get the y-component of the end-to-end distance of the walk.
template <class T>
double SpecWalker<T>::ry() const
{
    return (points().front() - points().back()).y();
}

/// Get the time where the minimum x-value is reached.
/// Attention! This is not well defined for integer walks
template <class T>
double SpecWalker<T>::argminx() const
{
    int argmin = 0;
    double min = 0; // we always start at (0,0), therefore this is a safe start
    auto &p = points();
    for(size_t i=0; i<p.size(); ++i)
    {
        if(p[i].x() < min)
        {
            min = p[i].x();
            argmin = i;
        }
    }
    return argmin;
}

/// Get the time where the maximum x-value is reached.
/// Attention! This is not well defined for integer walks
template <class T>
double SpecWalker<T>::argmaxx() const
{
    int argmax = 0;
    double max = 0; // we always start at (0,0), therefore this is a safe start
    auto &p = points();
    for(size_t i=0; i<p.size(); ++i)
    {
        if(p[i].x() > max)
        {
            max = p[i].x();
            argmax = i;
        }
    }
    return argmax;
}

/// Get the minimum x-value reached.
template <class T>
double SpecWalker<T>::minx() const
{
    T min = 0; // we always start at (0,0), therefore this is a safe start
    auto &p = points();
    for(size_t i=0; i<p.size(); ++i)
        if(p[i].x() < min)
            min = p[i].x();
    return min;
}

/// Get the maximum x-value reached.
template <class T>
double SpecWalker<T>::maxx() const
{
    T max = 0; // we always start at (0,0), therefore this is a safe start
    auto &p = points();
    for(size_t i=0; i<p.size(); ++i)
        if(p[i].x() > max)
            max = p[i].x();
    return max;
}

/// Get the squared end-to-end distance of the walk.
template <class T>
double SpecWalker<T>::r2() const
{
    double tmp = r();
    return tmp * tmp;
}

/// Get the number of vertices of the hull
template <class T>
int SpecWalker<T>::num_on_hull() const
{
    return convexHull().num_vertices();
}

/// Get the time at which the sign changes after starting at t1
/// return -1 if no sign change is detected for the remainder of the Walk
template <class T>
int SpecWalker<T>::passage(int t1, int axis) const
{
    if(t1 >= numSteps)
        return -1;
    int startSign = sign(points()[t1].x(axis));
    // if we are on zero, search the next non-null coordinate
    while(startSign == 0)
    {
        if(t1 >= numSteps)
            return -1;
        startSign = sign(points()[++t1].x(axis));
    }
    for(int i=t1+1; i<numSteps; ++i)
    {
        int nextSign = sign(points()[i].x(axis));
        if(nextSign != 0 && nextSign != startSign)
        {
            return i;
        }
    }
    return -1;
}

/** Get position in one direction at the specified timepoints.
 *
 * Can be used to calculate a correlation function.
 * \f[ \left< x_{t_1} x_{t_2} \right> - \left< x_{t_1} \right>\left< x_{t_2} \right> \f]
 *
 * \param t    vector of timepoints
 * \param axis component of the steps that should be outputted
 * \returns vector of the coordinate components
 */
template <class T>
std::vector<double> SpecWalker<T>::correlation(std::vector<int> t, int axis) const
{
    std::vector<double> out;
    if(*std::max_element(t.begin(), t.end()) >= numSteps)
    {
        LOG(LOG_ERROR) << "asked for a too big value (t_max = " << *std::max_element(t.begin(), t.end()) << " > " << numSteps << " = T)";
        exit(1);
    }
    for(auto i : t)
    {
        out.push_back(points()[i].x(axis));
    }

    return out;
}

/** Performs a greedy downhill optimization to maximize of minimize a
 * the given observable of a walk.
 *
 * \param maximize   Determines if maximization or minimization is carried out.
 * \param observable Which observable should be optimized.
 * \param stagnate   Abortion criterion, if stagnate many changes yielded no
                     improvement, finish simulation.
 */
template <class T>
void SpecWalker<T>::goDownhill(const bool maximize, const wanted_observable_t observable, const int stagnate)
{
    std::function<double()> S;
    if(observable == WO_SURFACE_AREA)
        S = [this](){ return this->L(); };
    if(observable == WO_VOLUME)
        S = [this](){ return this->A(); };

    while(true)
    {
        double veryOldS = S();
        // abort if there is no improvement after 1000 changes
        for(int i=0; i<stagnate; ++i)
        {
            // change one random number to another random number
            double oldS = S();
            change(rng);

            if(maximize ^ (S() > oldS))
                undoChange();
        }
        if(maximize ? S() <= veryOldS + 1e-5 : S() >= veryOldS - 1e-5)
            break;
        veryOldS = S();
    }
}

#endif
