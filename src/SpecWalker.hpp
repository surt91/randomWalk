#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "io.hpp"
#include "Cmd.hpp"
#include "Svg.hpp"
#include "RNG.hpp"
#include "Step.hpp"
#include "Povray.hpp"
#include "Walker.hpp"
#include "ConvexHullQHull.hpp"
#include "ConvexHullAndrew.hpp"
#include "ConvexHullJarvis.hpp"


/* Class Template SpecWalker.
 *
 * Implements generic things offered by the Walker interface.
 * Is Base Class of all Walker Implementations.
 * */
template <class T>
class SpecWalker : public Walker
{
    public:
        SpecWalker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : Walker(d, numSteps, rng, hull_algo),
              m_points(numSteps+1, Step<T>(d))
        {
        }

        SpecWalker(std::string data)
            : Walker(data)
        {
        }

        virtual ~SpecWalker() {}

        const ConvexHull<T>& convexHull() const;

        // convenience functions
        double A() const { return convexHull().A(); };
        double L() const { return convexHull().L(); };

        // get state
        virtual const std::vector<Step<T>>& steps() const = 0;
        const std::vector<Step<T>>& points(int start=1) const;
        const std::vector<Step<T>> hullPoints() const { return convexHull().hullPoints(); };

        // update state
        void updateSteps() const { steps(); };
        void updatePoints(int start=1) const { points(start); };
        void updateHull() const { convexHull(); };

        // output functions
        void svg(const std::string filename, const bool with_hull) const;
        void pov(const std::string filename, const bool with_hull) const;
        std::string print() const;

    protected:
        mutable std::vector<Step<T>> m_steps;
        mutable std::vector<Step<T>> m_points;
        mutable std::unique_ptr<ConvexHull<T>> m_convex_hull;
};

template <class T>
const ConvexHull<T>& SpecWalker<T>::convexHull() const
{
    if(hullDirty)
    {
        bool akl = false;
        switch(hull_algo)
        {
            case CH_QHULL_AKL:
                akl = true;
            case CH_QHULL:
                m_convex_hull = std::unique_ptr<ConvexHull<T>>(new ConvexHullQHull<T>(points(), akl));
                break;
            case CH_ANDREWS_AKL:
                akl = true;
            case CH_ANDREWS:
                m_convex_hull = std::unique_ptr<ConvexHull<T>>(new ConvexHullAndrew<T>(points(), akl));
                break;
            case CH_JARVIS_AKL:
                akl = true;
            case CH_JARVIS:
                m_convex_hull = std::unique_ptr<ConvexHull<T>>(new ConvexHullJarvis<T>(points(), akl));
                break;
            default:
                LOG(LOG_ERROR) << "Algorithm not implemented, yet: " << CH_LABEL[hull_algo];
                throw std::invalid_argument("this is not implemented");
        }
        hullDirty = false;
    }

    return *m_convex_hull;
}

template <class T>
const std::vector<Step<T>>& SpecWalker<T>::points(int start) const
{
    if(stepsDirty)
        steps();
    if(!pointsDirty && start!=1)
        return m_points;

    for(int i=start; i<=numSteps; ++i)
    {
        m_points[i].setZero();
        m_points[i] += m_points[i-1];
        m_points[i] += m_steps[i-1];
    }

    return m_points;
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
        const std::vector<Step<T>> h = convexHull().hullPoints();
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
std::string SpecWalker<T>::print() const
{
    std::stringstream ss;
    for(auto i : points())
        ss << i << " ";
    ss << "\n";
    return ss.str();
}
