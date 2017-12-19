#ifndef WALKER_H
#define WALKER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "../io.hpp"
#include "../Cmd.hpp"
#include "../RNG.hpp"

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
        Walker(int d, int numSteps, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);
        Walker(const Walker &) = default;
        virtual ~Walker() = default;

        const int numSteps;  ///< Number of steps the Walk should have
        const int d;         ///< Dimension in which the Walker walks

        virtual void reconstruct() = 0;
        virtual void generate_independent_sample() = 0;

        virtual void setHullAlgo(hull_algorithm_t a) = 0;

        virtual void setP1(double /*p1*/) { LOG(LOG_WARNING) << "P1 not used for this type of random walk"; }
        virtual void setP2(double /*p2*/) { LOG(LOG_WARNING) << "P2 not used for this type of random walk"; }

        // convenience functions
        virtual double A() const = 0;   ///< Returns the Volume of the convex hull
        virtual double L() const = 0;   ///< Returns the surface area of the convex hull
        virtual double maxDiameter() const = 0;            ///< Returns the maximum distance between all pairs
        virtual double r() const = 0;  ///< Distance between start and end point
        virtual double r2() const = 0; ///< Squared distance between start and end point
        virtual double rx() const = 0; ///< x coordinate of endpoint
        virtual double ry() const = 0; ///< y coordinate of endpoint
        virtual int num_on_hull() const = 0; ///< number of vertices of the hull
        virtual int passage(int t1=0, int axis=0) const = 0; ///< first passage of x=0 after t1
        virtual std::vector<double> correlation(std::vector<int> t, int axis=0) const = 0; ///< output a vector of points to calculate a correlation later

        /** Change the Walker by a small amount, appropiate for the type.
         *
         * Ensure that SpecWalker<T>::m_steps, SpecWalker<T>::m_points
         * and SpecWalker<T>::m_convex_hull are correct
         * afterwards, call updateSteps(), updatePoints() or
         * updateHull() or doing it manually in the implementation.
         */
        virtual void change(UniformRNG &rng, bool update=true) = 0;
        virtual void undoChange() = 0;

        virtual void updateSteps() = 0;
        virtual void updatePoints(int start=1) = 0;
        virtual void updateHull() = 0;

        virtual void degenerateMaxVolume() = 0;
        virtual void degenerateMaxSurface() = 0;
        virtual void degenerateMinVolume() = 0;
        virtual void degenerateMinSurface() = 0;

        virtual int nRN() const;

        ///\name serialization
        std::string serialize();
        void saveConfiguration(const std::string &filename, bool append=true);

        ///\name visualization
        virtual std::string print() const = 0;
        virtual void svg(const std::string filename, const bool with_hull=false) const = 0;
        virtual void pov(const std::string filename, const bool with_hull=false) const = 0;
        virtual void gp(const std::string filename, const bool with_hull=false) const = 0;
        virtual void threejs(const std::string filename, const bool with_hull=false) const = 0;

        virtual void goDownhill(const bool maximize, const wanted_observable_t observable, const int stagnate=1000) = 0;

    protected:
        UniformRNG rng;
        mutable std::vector<double> random_numbers;
        hull_algorithm_t hull_algo;
        bool amnesia; ///< if true, will not remember used random numbers, useful for non memory intensive simple sampling

        int undo_index;
        double undo_value;
};

#endif
