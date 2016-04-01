#pragma once

#include <set>
#include <memory>

#include "Logging.hpp"
#include "Step.hpp"

class ConvexHull
{
    public:
        ConvexHull(const std::vector<Step>& interiorPoints, bool akl);
        virtual ~ConvexHull();

        virtual double A() const = 0;
        virtual double L() const = 0;

        virtual const std::vector<Step>& hullPoints() const = 0;
        virtual std::vector<std::vector<Step>> hullFacets() const;

        // observables
        //~ virtual std::vector<int> max_extent();
        //~ virtual double diameter();

    protected:
        std::vector<Step> interiorPoints;
        mutable std::vector<Step> hullPoints_;
        int n;
        int d;

        virtual void preprocessAklToussaint();
};
