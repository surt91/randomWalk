#pragma once

#include <iostream>
#include <memory>

#include "Cmd.hpp"
#include "Svg.hpp"
#include "RNG.hpp"
#include "Step.hpp"
#include "ConvexHullQHull.hpp"
#include "ConvexHullAndrew.hpp"

/* Class Walker
 *
 * Saves a vector of random numbers [0,1] and generates a random walk
 * on demand.
 *
 * Also exhibits functions to change random numbers in that
 * vector and convinience functions to calculate the convex hull
 * of the walk.
 * */
class Walker
{
    public:
        Walker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
            : numSteps(numSteps),
              d(d),
              rng(rng),
              random_numbers(rng.vector(numSteps)),
              hull_algo(hull_algo)
        {
            stepsDirty = true;
            pointsDirty = true;
            hullDirty = true;
        };

        virtual ~Walker() {};

        const Step& position() const;

        void appendRN();

        virtual double rnChange(const int idx, const double other);

        const ConvexHull& convexHull() const;
        // convenience functions
        double A() const { return convexHull().A(); };
        double L() const { return convexHull().L(); };
        const std::vector<Step> hullPoints() const { return convexHull().hullPoints(); };

        const std::vector<Step>& points(int start=1) const;
        virtual const std::vector<Step> steps() const;

        int nSteps() const;

        std::string print() const;
        void svg(const std::string filename, const bool with_hull=false) const;

    protected:
        int numSteps;
        mutable int stepsDirty;
        mutable int pointsDirty;
        mutable int hullDirty;
        mutable std::vector<Step> m_steps;
        mutable std::vector<Step> m_points;
        mutable std::unique_ptr<ConvexHull> m_convex_hull;

        int d;
        UniformRNG rng;
        mutable std::vector<double> random_numbers;
        hull_algorithm_t hull_algo;
};
