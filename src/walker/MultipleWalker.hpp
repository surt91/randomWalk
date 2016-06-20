#pragma once

#include <memory>

#include "../Cmd.hpp"
#include "../visualization/Svg.hpp"
#include "../visualization/Povray.hpp"
#include "../visualization/Gnuplot2D.hpp"
#include "../visualization/Gnuplot3D.hpp"
#include "../RNG.hpp"
#include "../Step.hpp"
#include "../ConvexHull.hpp"
#include "Walker.hpp"

/// Colors used by the SVG backend for different walks
static const std::vector<std::string> COLOR = {
    "green",
    "blue",
    "yellow",
    "orange",
    "indigo",
    "cyan",
    "brown",
    "crimson",
    "darkmagenta",
    "deepskyblue"
};

/** Class Template MultipleWalker.
 *
 * Implements a generic way to represent multiple Walkers of the
 * type of the template parameter T. To the outside it will look
 * like one Walk and offers the same interface as the generic
 * Walker class.
 *
 * Advantages:
 *   + directly available for all subclasses of Walker
 *   + no influence on single walker
 *      -> no code needs to be touched
 *      -> no performance regressions are possible
 *
 * \tparam T class of the Walk type that should be wrapped
 */
template <class T>
class MultipleWalker : public Walker
{
    public:
        MultipleWalker(int d, int numSteps, int numWalkers, UniformRNG &rng, hull_algorithm_t hull_algo);
        virtual ~MultipleWalker() {}

        int numWalker;

        //\name implementing pure virtual functions
        virtual void setHullAlgo(hull_algorithm_t a);

        virtual void setP1(double p1);
        virtual void setP2(double p2);

        // convenience functions
        double A() const final;
        double L() const final;
        std::vector<double> maxExtent() final;
        double maxDiameter() final;
        double r() final;
        double rx() final;
        double ry() final;
        double r2() final;

        virtual void change(UniformRNG &rng, bool update=true);
        virtual void undoChange();

        virtual void updateSteps();
        virtual void updatePoints(int start=1);
        virtual void updateHull();

        virtual void degenerateMaxVolume();
        virtual void degenerateMaxSurface();
        virtual void degenerateMinVolume();
        virtual void degenerateMinSurface();

        virtual int nRN() const;

        ///\name visualization
        virtual std::string print() const;
        virtual void svg(const std::string filename, const bool with_hull=false) const;
        virtual void pov(const std::string filename, const bool with_hull=false) const;
        virtual void gp(const std::string filename, const bool with_hull=false) const;

    protected:
        std::vector<T> m_walker;

        ConvexHull<decltype(T::T_type())> m_convex_hull;
        ConvexHull<decltype(T::T_type())> m_old_convex_hull;
        int undo_walker_idx;
};

template <class T>
MultipleWalker<T>::MultipleWalker(int d, int numSteps, int numWalker, UniformRNG &rng, hull_algorithm_t hull_algo)
    : Walker(d, numSteps, rng, hull_algo),
      numWalker(numWalker)
{
    m_walker.reserve(numWalker);
    for(int i=0; i<numWalker; ++i)
        m_walker.emplace_back(d, numSteps, rng, hull_algo);
}

template <class T>
void MultipleWalker<T>::setHullAlgo(hull_algorithm_t a)
{
    for(auto w : m_walker)
        w.setHullAlgo(a);
}

template <class T>
void MultipleWalker<T>::setP1(double p1)
{
    for(auto w : m_walker)
        w.setP1(p1);
}

template <class T>
void MultipleWalker<T>::setP2(double p2)
{
    for(auto w : m_walker)
        w.setP2(p2);
}

template <class T>
void MultipleWalker<T>::change(UniformRNG &rng, bool update)
{
    undo_walker_idx = floor(rng() * m_walker.size());
    // change a random walker, but do not update
    // since we will calculate the hull of every walker here
    m_walker[undo_walker_idx].change(rng, false);

    if(update)
    {
        m_old_convex_hull = m_convex_hull;
        updateHull();
    }
}

template <class T>
void MultipleWalker<T>::undoChange()
{
    m_walker[undo_walker_idx].undoChange();
    m_convex_hull = m_old_convex_hull;
}

template <class T>
void MultipleWalker<T>::updateSteps()
{
    for(auto w : m_walker)
        w.updateSteps();
}

// We ignore start, since this parameter should only be used from inside
// the class or at least most often.
template <class T>
void MultipleWalker<T>::updatePoints(int /*start*/)
{
    for(auto w : m_walker)
        w.updatePoints();
}

template <class T>
void MultipleWalker<T>::updateHull()
{
    // TODO
    // Is it faster to calculate the hull of the union of all points
    // of all walkers or to keep the hulls of the single walkers up
    // to date and calculate the hull of the union of the hulls of
    // all walkers?

    //~ for(auto w : m_walker)
        //~ w.updateHull();


    //~ hulls.push_back(m_walker[0].convexHull())
    //~ for(auto w : m_walker)
        //~ w.convexHull();

    // FIXME: this will be coping way too much and needs to be
    // optimized. Maybe with a new method of ConvexHull which takes
    // multiple vectors?
    // omg this seems not right
    std::vector<Step<decltype(T::T_type())>> all_points;
    all_points.reserve(numSteps*numWalker);

    for(auto w : m_walker)
    {
        auto &p = w.points();
        all_points.insert(all_points.end(), p.begin(), p.end());
    }
    m_convex_hull.run(&all_points);

    LOG(LOG_INFO) << "Updated";
    LOG(LOG_INFO) << m_convex_hull.A() << " " << m_convex_hull.L();
}

template <class T>
double MultipleWalker<T>::A() const { return m_convex_hull.A(); }
template <class T>
double MultipleWalker<T>::L() const { return m_convex_hull.L(); }
template <class T>
std::vector<double> MultipleWalker<T>::maxExtent() { LOG(LOG_WARNING) << "not yet implemented"; return {0}; }
template <class T>
double MultipleWalker<T>::maxDiameter() { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::r() { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::rx() { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::ry() { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::r2() { LOG(LOG_WARNING) << "not yet implemented"; return 0; }

template <class T>
void MultipleWalker<T>::degenerateMaxVolume() { LOG(LOG_WARNING) << "not yet implemented"; }
template <class T>
void MultipleWalker<T>::degenerateMaxSurface() { LOG(LOG_WARNING) << "not yet implemented"; }
template <class T>
void MultipleWalker<T>::degenerateMinVolume() { LOG(LOG_WARNING) << "not yet implemented"; }
template <class T>
void MultipleWalker<T>::degenerateMinSurface() { LOG(LOG_WARNING) << "not yet implemented"; }

template <class T>
int MultipleWalker<T>::nRN() const
{
    int n = 0;
    for(auto w : m_walker)
        n += w.nRN();
    return n;
}

template <class T>
std::string MultipleWalker<T>::print() const
{
    std::stringstream ss;

    for(auto w : m_walker)
        ss << "[" << w.print() << "]\n";
    ss << "\n";
    return ss.str();
}

// FIXME: this is almost exactly the same code as in SpecWalker
// can probably be reduced
template <class T>
void MultipleWalker<T>::svg(const std::string filename, const bool with_hull) const
{
    SVG pic(filename);
    int min_x=0, max_x=0, min_y=0, max_y=0;
    int idx = 0;
    for(auto w : m_walker)
    {
        ++idx;
        std::vector<std::vector<double>> points;
        const auto p = w.points();

        for(auto i : p)
        {
            auto x1 = i[0], y1 = i[1];
            std::vector<double> point {(double) x1, (double) y1};

            pic.circle(x1, y1, true, COLOR[idx%COLOR.size()]);

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
        pic.polyline(points, false, COLOR[idx%COLOR.size()]);
    }

    if(d > 2)
        pic.text(min_x, max_y-20, "projected from d=" + std::to_string(d), "red");

    if(with_hull)
    {
        std::vector<std::vector<double>> points;
        const auto h = m_convex_hull.hullPoints();
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

// FIXME: this is almost exactly the same code as in SpecWalker
// can probably be reduced
template <class T>
void MultipleWalker<T>::pov(const std::string /*filename*/, const bool /*with_hull*/) const { LOG(LOG_WARNING) << "not yet implemented"; }
template <class T>
void MultipleWalker<T>::gp(const std::string filename, const bool with_hull) const
{
    if(d==2)
    {
        Gnuplot2D pic(filename);

        int idx = 0;
        for(auto w : m_walker)
        {
            ++idx;
            const auto p = w.points();
            std::vector<std::vector<double>> points;
            for(auto i : p)
            {
                auto x = i[0], y = i[1];
                std::vector<double> point {(double) x, (double) y};

                points.push_back(point);
            }
            pic.polyline(points, false);
        }

        if(with_hull)
        {
            std::vector<std::vector<double>> points;
            const auto h = m_convex_hull.hullPoints();
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
        int idx = 0;
        for(auto w : m_walker)
        {
            ++idx;
            auto p = w.points();
            std::vector<std::vector<double>> points;
            for(auto i : p)
            {
                auto x = i[0], y = i[1], z = i[2];
                std::vector<double> point {(double) x, (double) y, (double) z};

                points.push_back(point);
            }
            pic.polyline(points);
        }

        if(with_hull)
        {
            for(auto &i : m_convex_hull.hullFacets())
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
