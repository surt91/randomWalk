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
#include "ConvexHullQHull.hpp"
#include "ConvexHullAndrew.hpp"
#include "ConvexHullJarvis.hpp"


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

        virtual ~Walker() {};

        const Step& position() const;

        void appendRN();

        virtual void change(UniformRNG &rng);
        virtual void undoChange();

        void degenerate();
        void degenerateMaxSurface();

        void setHullAlgo(hull_algorithm_t a);

        const ConvexHull& convexHull() const;
        // convenience functions
        const std::vector<Step> hullPoints() const { return convexHull().hullPoints(); };
        double A() const { return convexHull().A(); };
        double L() const { return convexHull().L(); };

        const std::vector<Step>& points(int start=1) const;
        virtual const std::vector<Step> steps() const;

        int nSteps() const;
        int nRN() const;

        std::string serialize();
        void deserialize(std::string s);

        void saveConfiguration(const std::string &filename, bool append=true);
        void loadConfiguration(const std::string &filename, int index=0);

        std::string print() const;
        void svg(const std::string filename, const bool with_hull=false) const;
        void pov(const std::string filename, const bool with_hull=false) const;

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

        int undo_index;
        double undo_value;
        //~ std::vector<Step> undo_points;
        //~ ConvexHull *undo_hull;
};
