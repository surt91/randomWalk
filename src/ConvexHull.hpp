#pragma once

#include <set>
#include <memory>

#include "Logging.hpp"
#include "Step.hpp"

/** Abstract base class templates of all Convex Hull class templates.
 *
 * Offers pure virtual functions to access observalbes of the convex
 * hull. Acts as an interface.
 */
template <class T>
class ConvexHull
{
    public:
        ConvexHull(const std::vector<Step<T>> &interiorPoints, bool /*akl*/)
            : interiorPoints(interiorPoints),
              n(interiorPoints.size()),
              d(interiorPoints[0].d())
        {
        }
        virtual ~ConvexHull() {}

        // observables
        virtual double A() const = 0;
        virtual double L() const = 0;
        //~ virtual std::vector<int> max_extent();
        //~ virtual double diameter();

        // hull
        virtual const std::vector<Step<T>>& hullPoints() const = 0;
        virtual std::vector<std::vector<Step<T>>> hullFacets() const
        {
            throw std::invalid_argument("hull facets for this algorithm not implemented");
        }

    protected:
        const std::vector<Step<T>>& interiorPoints;
        mutable std::vector<Step<T>> hullPoints_;
        int n;
        int d;

        mutable double m_A;
        mutable double m_L;

        // deletes points from interior points according to the Akl Toussaint heuristic
        virtual void preprocessAklToussaint()
        {
            throw std::invalid_argument("Akl Toussaint Heuristic currently only implemented for d=2");
        }
};
