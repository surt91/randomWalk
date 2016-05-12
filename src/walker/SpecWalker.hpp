#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "../io.hpp"
#include "../Cmd.hpp"
#include "../visualization/Svg.hpp"
#include "../visualization/Povray.hpp"
#include "../visualization/Gnuplot.hpp"
#include "../RNG.hpp"
#include "../Step.hpp"
#include "../ConvexHull.hpp"
#include "Walker.hpp"

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

        const ConvexHull<T>& convexHull() const;
        void setHullAlgo(hull_algorithm_t a);

        ///\name observables
        double A() const final { return convexHull().A(); };
        double L() const final { return convexHull().L(); };
        std::vector<double> maxExtent() final;
        double maxDiameter() final ;
        double r() final ;
        double r2() final;

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
        virtual void degenerateSpiral();
        virtual void degenerateStraight();

    protected:
        std::vector<Step<T>> m_steps;
        std::vector<Step<T>> m_points;
        ConvexHull<T> m_convex_hull;
};

/// do initialization, e.g. calculate the steps and the hull
template <class T>
void SpecWalker<T>::init()
{
    updateSteps();
    updatePoints();
    setHullAlgo(hull_algo); // does also update hull
}

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
void SpecWalker<T>::setHullAlgo(hull_algorithm_t a)
{
    m_convex_hull.setHullAlgo(a);
    updateHull();
    hull_algo = a;
}

template <class T>
void SpecWalker<T>::updatePoints(int start)
{
    for(int i=start; i<=numSteps; ++i)
    {
        m_points[i].setZero();
        m_points[i] += m_points[i-1];
        m_points[i] += m_steps[i-1];
    }
}

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

template <class T>
void SpecWalker<T>::gp(const std::string filename, const bool with_hull) const
{
    Gnuplot pic(filename);
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

template <class T>
std::string SpecWalker<T>::print() const
{
    std::stringstream ss;
    for(auto i : points())
        ss << i << " ";
    ss << "\n";
    return ss.str();
}

template <class T>
std::vector<double> SpecWalker<T>::maxExtent()
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

template <class T>
double SpecWalker<T>::maxDiameter()
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

template <class T>
double SpecWalker<T>::r()
{
    return (points().front() - points().back()).length();
}

template <class T>
double SpecWalker<T>::r2()
{
    double tmp = r();
    return tmp * tmp;
}

/** set the random numbers such that we get an L shape
 */
template <>
inline void SpecWalker<int>::degenerateMaxVolume()
{
    for(int i=0; i<numSteps; ++i)
        random_numbers[i] = .99 / ceil((double) d * (i+1)/numSteps);

    updateSteps();
    updatePoints();
    updateHull();
}

/** set the random numbers such that we get an L shape in d-1 dimensions
 */
template <>
inline void SpecWalker<int>::degenerateMaxSurface()
{
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99 / ceil((double) (d-1) * (i+1)/random_numbers.size());

    updateSteps();
    updatePoints();
    updateHull();
}

/** set the random numbers such that we get a spiral
 */
template <>
inline void SpecWalker<int>::degenerateSpiral()
{
    // TODO: find some easy construction for a spiral in arbitrary dimensions
    // FIXME: right now, it is a straight line instead of a spiral
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99;

    updateSteps();
    updatePoints();
    updateHull();
}

/** set the random numbers such that we get a straight line
 */
template <>
inline void SpecWalker<int>::degenerateStraight()
{
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99;

    updateSteps();
    updatePoints();
    updateHull();
}

/** Set the random numbers such that we get an half circle shape.
 */
template <>
inline void SpecWalker<double>::degenerateMaxVolume()
{
    // FIXME: this works only for d=2
    for(int i=0; i<numSteps; ++i)
        random_numbers[i] = .5 / (i+1);

    updateSteps();
    updatePoints();
    updateHull();
}

/** Set the random numbers such that we get an L shape in d-1 dimensions.
 */
template <>
inline void SpecWalker<double>::degenerateMaxSurface()
{
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99 / ceil((double) (d-1) * (i+1)/random_numbers.size());

    updateSteps();
    updatePoints();
    updateHull();
}

/** Set the random numbers such that we get a spiral.
 */
template <>
inline void SpecWalker<double>::degenerateSpiral()
{
    // TODO: find some easy construction for a spiral in arbitrary dimensions
    // FIXME: right now, it is a straight line instead of a spiral
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99;

    updateSteps();
    updatePoints();
    updateHull();
}

/** Set the random numbers such that we get a straight line.
 */
template <>
inline void SpecWalker<double>::degenerateStraight()
{
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99;

    updateSteps();
    updatePoints();
    updateHull();
}
