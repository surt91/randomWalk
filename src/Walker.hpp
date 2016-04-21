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
        }

        Walker(std::string data)
        {
            stepsDirty = true;
            pointsDirty = true;
            hullDirty = true;

            deserialize(data);
        }

        void appendRN();

        void setHullAlgo(hull_algorithm_t a);

        //~ const ConvexHull& convexHull() const;
        // convenience functions
        //~ const std::vector<Step> hullPoints() const { return convexHull().hullPoints(); };
        virtual double A() const = 0;
        virtual double L() const = 0;

        virtual void change(UniformRNG &rng) = 0;
        virtual void undoChange() = 0;

        virtual void updateSteps() const = 0;
        virtual void updateHull() const = 0;
        //~ virtual const std::vector<Step<T>>& points(int start=1) const = 0;

        virtual void degenerateMaxVolume() = 0;
        virtual void degenerateMaxSurface() = 0;
        virtual void degenerateSpiral() = 0;
        virtual void degenerateStraight() = 0;

        virtual int nSteps() const = 0;
        int nRN() const;

        std::string serialize();
        void deserialize(std::string s);

        void saveConfiguration(const std::string &filename, bool append=true);
        void loadConfiguration(const std::string &filename, int index=0);

        virtual std::string print() const = 0;
        virtual void svg(const std::string filename, const bool with_hull=false) const = 0;
        virtual void pov(const std::string filename, const bool with_hull=false) const = 0;

    protected:
        int numSteps;
        mutable int stepsDirty;
        mutable int pointsDirty;
        mutable int hullDirty;

        int d;
        UniformRNG rng;
        mutable std::vector<double> random_numbers;
        hull_algorithm_t hull_algo;

        int undo_index;
        double undo_value;
};
