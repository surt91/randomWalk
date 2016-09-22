#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "../io.hpp"
#include "../Cmd.hpp"
#include "../visualization/Svg.hpp"
#include "../visualization/Povray.hpp"
#include "../visualization/Gnuplot2D.hpp"
#include "../visualization/Gnuplot3D.hpp"
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
        SpecWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : Walker(d, numSteps, rng, hull_algo),
              m_points(numSteps+1, Step<T>(d))
        {
        }

        virtual ~SpecWalker() {}

        void init();
        virtual void reconstruct();

        const ConvexHull<T>& convexHull() const;
        void setHullAlgo(hull_algorithm_t a);

        /// function to make the type of T accessable outside (e.g. per decltype)
        static T T_type() { return T(); };

        ///\name observables
        double A() const final { return convexHull().A(); };
        double L() const final { return convexHull().L(); };
        std::vector<double> maxExtent() const final;
        double maxDiameter() const final;
        double r() const final;
        double r2() const final;
        double rx() const final;
        double ry() const final;
        int passage(int t1=0) const final;

        ///\name get state
        const std::vector<Step<T>>& steps() const { return m_steps; };
        const std::vector<Step<T>>& points() const { return m_points; };
        const std::vector<Step<T>>& hullPoints() const { return convexHull().hullPoints(); };

        ///\name update state
        virtual void updateSteps() override = 0;
        virtual void updatePoints(int start=1) override;
        virtual void updateHull() override;

        ///\name visualization
        void svg(const std::string filename, const bool with_hull) const final;
        void pov(const std::string filename, const bool with_hull) const final;
        void gp(const std::string filename, const bool with_hull) const final;
        std::string print() const final;

        ///\name degenerate cases
        virtual void degenerateMaxVolume();
        virtual void degenerateMaxSurface();
        virtual void degenerateMinVolume();
        virtual void degenerateMinSurface();

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
    std::generate(random_numbers.begin(), random_numbers.end(), [this]{ return this->rng(); });
    init();
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
            T x = i[0], y = i[1], z = 0;
            if(d > 2)
                z = i[2];
            std::vector<double> point {(double) x, (double) y, (double) z};

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

/// Get the maximum extend along any axis.
template <class T>
std::vector<double> SpecWalker<T>::maxExtent() const
{
    std::vector<double> maxE(d, 0);
    int n_hullpoints = hullPoints().size();
    Step<T> diff;
    for(int i=0; i<n_hullpoints; ++i)
        for(int j=0; j<i; ++j)
        {
            diff = hullPoints()[i] - hullPoints()[j];
            for(int k=0; k<d; ++k)
                if(std::abs(diff[k]) > maxE[k])
                    maxE[k] = std::abs(diff[k]);
        }

    return maxE;
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

/// Get the squared end-to-end distance of the walk.
template <class T>
double SpecWalker<T>::r2() const
{
    double tmp = r();
    return tmp * tmp;
}

/// Get the time at which the sign changes after starting at t1
/// return -1 if no sign change is detected for the remainder of the Walk
template <class T>
int SpecWalker<T>::passage(int t1) const
{
    if(t1 >= numSteps)
        return -1;
    int startSign = sign(points()[t1].x());
    // if we are on zero, search the next non-null coordinate
    while(startSign == 0)
    {
        if(t1 >= numSteps)
            return -1;
        startSign = sign(points()[++t1].x());
    }
    for(int i=t1+1; i<numSteps; ++i)
    {
        int nextSign = sign(points()[i].x());
        if(nextSign != 0 && nextSign != startSign)
        {
            return i;
        }
    }
    return -1;
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

/// Set the random numbers such that we get an L shape.
template <>
inline void SpecWalker<int>::degenerateMaxVolume()
{
    for(int i=0; i<numSteps; ++i)
        random_numbers[i] = .99 / ceil((double) d * (i+1)/numSteps);

    updateSteps();
    updatePoints();
    updateHull();
}

/// Set the random numbers such that we get an L shape in d-1 dimensions.
template <>
inline void SpecWalker<int>::degenerateMaxSurface()
{
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99 / ceil((double) (d-1) * (i+1)/random_numbers.size());

    updateSteps();
    updatePoints();
    updateHull();
}

/// Set the random numbers such that we get an one dimensional line.
template <>
inline void SpecWalker<int>::degenerateMinVolume()
{
    for(int i=0; i<numSteps; ++i)
        random_numbers[i] = .99;

    updateSteps();
    updatePoints();
    updateHull();
}

/// Set the random numbers such that we always step left, right, left, right.
template <>
inline void SpecWalker<int>::degenerateMinSurface()
{
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = i % 2 ? .99 : .99 - 1./d;

    updateSteps();
    updatePoints();
    updateHull();
}

/// Set the random numbers such that we get an half circle shape.
template <>
inline void SpecWalker<double>::degenerateMaxVolume()
{
    // FIXME: this works only for d=2
    if(d>2)
    {
        LOG(LOG_WARNING) << "Max Volume configuration is not really max volume";
    }
    for(int i=0; i<numSteps; ++i)
        random_numbers[i] = .5 / (i+1);

    updateSteps();
    updatePoints();
    updateHull();

    goDownhill(true, WO_VOLUME);
}

/// Set the random numbers such that we get an half circle shape in d-1 dimensions.
template <>
inline void SpecWalker<double>::degenerateMaxSurface()
{
    // FIXME: this works only for d<=3
    if(d>3)
    {
        LOG(LOG_WARNING) << "Max Surface configuration is not really max surface";
    }
    if(d==3)
        for(size_t i=0; i<random_numbers.size(); ++i)
            random_numbers[i] = .99 / ceil((double) (d-1) * (i+1)/random_numbers.size());
    if(d==2)
        for(size_t i=0; i<random_numbers.size(); ++i)
            random_numbers[i] = .99;

    updateSteps();
    updatePoints();
    updateHull();

    goDownhill(true, WO_SURFACE_AREA);
}

/// Set the random numbers such that we get an half circle shape in d-1 dimensions.
template <>
inline void SpecWalker<double>::degenerateMinVolume()
{
    // FIXME: this works only for d=2
    for(int i=0; i<numSteps; ++i)
        random_numbers[i] = .5 / (i+1);

    updateSteps();
    updatePoints();
    updateHull();

    goDownhill(false, WO_VOLUME);
}

/// Set the random numbers such that we get an L shape in d-1 dimensions.
template <>
inline void SpecWalker<double>::degenerateMinSurface()
{
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99 / ceil((double) (d-1) * (i+1)/random_numbers.size());

    updateSteps();
    updatePoints();
    updateHull();

    goDownhill(false, WO_SURFACE_AREA);
}
