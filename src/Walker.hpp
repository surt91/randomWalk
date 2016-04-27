#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "io.hpp"
#include "Cmd.hpp"
#include "Svg.hpp"
#include "RNG.hpp"
#include "Povray.hpp"


/** Abstract base class for all Walker classes.
 *
 * Offers an interface to access generic functions to manipulate,
 * visualize and get observables the Walker and its hull.
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
              hull_algo(hull_algo)
        {
            stepsDirty = true;
            pointsDirty = true;
            hullDirty = true;
        }

        virtual ~Walker() {}

        const int numSteps; ///< Number of steps the Walk should have
        const int d;        ///< Dimension in which the Walker walks

        virtual void setHullAlgo(hull_algorithm_t a) = 0;

        // convenience functions
        virtual double A() const = 0;   ///< Returns the Volume of the convex hull
        virtual double L() const = 0;   ///< Returns the surface area of the convex hull

        /** Change the Walker by a small amount, appropiate for the type.
         *
         * Ensure that SpecWalker<T>::m_steps, SpecWalker<T>::m_points
         * and SpecWalker<T>::m_convex_hull are correct
         * afterwards, by either setting #stepsDirty, #pointsDirty and
         * #hullDirty to true, call updateSteps(), updatePoints() or
         * updateHull() or doing it manually in the implementation.
         */
        virtual void change(UniformRNG &rng) = 0;
        virtual void undoChange() = 0;

        virtual void updateSteps() const = 0;
        virtual void updatePoints(int start) const = 0;
        virtual void updateHull() const = 0;

        virtual void degenerateMaxVolume() = 0;
        virtual void degenerateMaxSurface() = 0;
        virtual void degenerateSpiral() = 0;
        virtual void degenerateStraight() = 0;

        virtual int nRN() const;

        std::string serialize();
        void saveConfiguration(const std::string &filename, bool append=true);

        virtual std::string print() const = 0;
        virtual void svg(const std::string filename, const bool with_hull=false) const = 0;
        virtual void pov(const std::string filename, const bool with_hull=false) const = 0;

    protected:
        mutable int stepsDirty;
        mutable int pointsDirty;
        mutable int hullDirty;

        UniformRNG rng;
        mutable std::vector<double> random_numbers;
        hull_algorithm_t hull_algo;

        int undo_index;
        double undo_value;
};
