#ifndef MULTIPLEWALKER_H
#define MULTIPLEWALKER_H

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
 *
 * \image html multi.svg "two random walks and their joint convex hull"
 */
template <class T>
class MultipleWalker : public Walker
{
    public:
        MultipleWalker(int d, int numSteps, int numWalkers, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);
        virtual ~MultipleWalker() {}

        int numWalker;

        virtual void reconstruct();
        virtual void generate_independent_sample();

        //\name implementing pure virtual functions
        virtual void setHullAlgo(hull_algorithm_t a);

        virtual void setP1(double p1);
        virtual void setP2(double p2);
        virtual void setP3(double p3);

        // convenience functions
        double A() const final;
        double L() const final;
        double maxDiameter() const final;
        double r() const final;
        double rx() const final;
        double ry() const final;
        double r2() const final;
        double argminx() const final;
        double argmaxx() const final;
        double minx() const final;
        double maxx() const final;
        int num_on_hull() const final;
        double oblateness() const final;
        double length() const final;
        int steps_taken() const final;
        int num_resets() const final;
        int maxsteps_partialwalk() const final;
        double maxlen_partialwalk() const final;
        int visitedSites() const final;
        int enclosedSites() const final;
        int passage(int t1=0, int axis=0) const final;
        std::vector<double> correlation(std::vector<int> t, int axis=0) const final;

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
        virtual void svgOfChange(std::string filename, UniformRNG &rng_in);
        virtual void pov(const std::string filename, const bool with_hull=false) const;
        virtual void gp(const std::string filename, const bool with_hull=false) const;
        virtual void threejs(const std::string filename, const bool with_hull=false) const;

        void goDownhill(const bool, const wanted_observable_t, const int ) {LOG(LOG_ERROR) << "not implemented";}

    protected:
        std::vector<T> m_walker;

        ConvexHull<decltype(T::T_type())> m_convex_hull;
        ConvexHull<decltype(T::T_type())> m_old_convex_hull;
        int undo_walker_idx;
};

template <class T>
MultipleWalker<T>::MultipleWalker(int d, int numSteps, int numWalker, const UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : Walker(d, numSteps, rng_in, hull_algo, amnesia),
      numWalker(numWalker)
{
    m_walker.reserve(numWalker);
    for(int i=0; i<numWalker; ++i)
    {
        auto rng_privat = UniformRNG(rng() * 2000000000);
        m_walker.emplace_back(d, numSteps, rng_privat, hull_algo, amnesia);
    }
    updateHull();
}

/// Get new random numbers and reconstruct the walk
template <class T>
void MultipleWalker<T>::reconstruct()
{
    for(auto &w : m_walker)
        w.reconstruct();
    updateHull();
}

/// For most Walks this will be just a simple reconstruct, but
/// e.g., SAW will need a MCMC equilibration with the pivot algorithm
/// which will be performed in this call
template <class T>
void MultipleWalker<T>::generate_independent_sample()
{
    for(auto &w : m_walker)
        w.generate_independent_sample();
    updateHull();
}

template <class T>
void MultipleWalker<T>::setHullAlgo(hull_algorithm_t a)
{
    for(auto &w : m_walker)
        w.setHullAlgo(a);
}

template <class T>
void MultipleWalker<T>::setP1(double p1)
{
    for(auto &w : m_walker)
        w.setP1(p1);
}

template <class T>
void MultipleWalker<T>::setP2(double p2)
{
    for(auto &w : m_walker)
        w.setP2(p2);
}

template <class T>
void MultipleWalker<T>::setP3(double p3)
{
    for(auto &w : m_walker)
        w.setP3(p3);
}

template <class T>
void MultipleWalker<T>::change(UniformRNG &rng, bool update)
{
    undo_walker_idx = floor(rng() * m_walker.size());
    m_walker[undo_walker_idx].change(rng, true);

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
    for(auto &w : m_walker)
        w.updateSteps();
}

// We ignore start, since this parameter should only be used from inside
// the class or at least most often.
template <class T>
void MultipleWalker<T>::updatePoints(int /*start*/)
{
    for(auto &w : m_walker)
        w.updatePoints();
}

template <class T>
void MultipleWalker<T>::updateHull()
{
    // FIXME: this will be copying way too much and needs to be
    // optimized. Maybe with a new method of ConvexHull which takes
    // multiple vectors?
    // omg this seems not right
    std::vector<Step<decltype(T::T_type())>> all_points;
    all_points.reserve(numSteps*numWalker);

    for(auto &w : m_walker)
    {
        auto &p = w.hullPoints();
        all_points.insert(all_points.end(), p.begin(), p.end());
    }
    m_convex_hull.run(&all_points);

    LOG(LOG_TOO_MUCH) << "Updated";
    LOG(LOG_TOO_MUCH) << m_convex_hull.A() << " " << m_convex_hull.L();
}

template <class T>
double MultipleWalker<T>::A() const { return m_convex_hull.A(); }
template <class T>
double MultipleWalker<T>::L() const { return m_convex_hull.L(); }
template <class T>
double MultipleWalker<T>::maxDiameter() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::r() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::rx() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::ry() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::r2() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::argminx() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::argmaxx() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::minx() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::maxx() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
int MultipleWalker<T>::num_on_hull() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::oblateness() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::length() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
int MultipleWalker<T>::steps_taken() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
int MultipleWalker<T>::num_resets() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
int MultipleWalker<T>::maxsteps_partialwalk() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
double MultipleWalker<T>::maxlen_partialwalk() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
int MultipleWalker<T>::visitedSites() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
int MultipleWalker<T>::enclosedSites() const { LOG(LOG_WARNING) << "not yet implemented"; return 0; }
template <class T>
int MultipleWalker<T>::passage(int, int) const {LOG(LOG_ERROR) << "not implemented"; return 0; }
template <class T>
std::vector<double> MultipleWalker<T>::correlation(std::vector<int>, int) const  {LOG(LOG_ERROR) << "not implemented"; return std::vector<double>(); }

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
    for(const auto &w : m_walker)
        n += w.nRN();
    return n;
}

template <class T>
std::string MultipleWalker<T>::print() const
{
    std::stringstream ss;

    for(const auto &w : m_walker)
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
    for(const auto &w : m_walker)
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

        points.clear();
        if(with_hull)
        {
            const auto h = w.hullPoints();
            for(auto &i : h)
            {
                std::vector<double> point {(double) i[0], (double) i[1]};
                points.push_back(point);
            }
            pic.polyline(points, true, COLOR[idx%COLOR.size()]);
        }
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

template <class T>
void MultipleWalker<T>::svgOfChange(std::string /*filename*/, UniformRNG &/*rng_in*/) { LOG(LOG_WARNING) << "not yet implemented"; }

// FIXME: this is almost exactly the same code as in SpecWalker
// can probably be reduced
template <class T>
void MultipleWalker<T>::pov(const std::string /*filename*/, const bool /*with_hull*/) const { LOG(LOG_WARNING) << "not yet implemented"; }
template <class T>
void MultipleWalker<T>::threejs(const std::string /*filename*/, const bool /*with_hull*/) const { LOG(LOG_WARNING) << "not yet implemented"; }
template <class T>
void MultipleWalker<T>::gp(const std::string filename, const bool with_hull) const
{
    if(d==2)
    {
        Gnuplot2D pic(filename);

        int idx = 0;
        for(const auto &w : m_walker)
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
        for(const auto &w : m_walker)
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

#endif
