#ifndef WANGLANDAU_H
#define WANGLANDAU_H

#include "Simulation.hpp"
#include "Histogram.hpp"

#include <iostream>
#include <fstream>

/** Wang Landau Sampling of the distribution of a given observable.
 *
 * Wang Landau sampling will sample the distribution over the full
 * support. Takes usually longer than Metropolis sampling, but does
 * not need parameters like temperatures or explicit equilibration.
 *
 * The idea is to estimate a density function \f$g\f$ on the fly
 * and using this function to accept or reject changes as
 * \f[ p_\mathrm{acc}(E_1 \to E_2) = \min\left(1, \frac{g(E_1)}{g(E_2)}\right). \f]
 * The estimate of \f$g\f$ is updated by multiplication with some refinement
 * parameter \f$f\f$, which is updated when \f$g\f$ is a sufficiently good
 * estimate of the real density function (i.e., the histogram of visited
 * energies is sufficiently flat).
 *
 * See http://arxiv.org/pdf/cond-mat/0011174.pdf
 */
class WangLandau : public Simulation
{
    public:
        WangLandau(const Cmd &o);
        virtual void run() override;

        static std::vector<std::vector<double>> generateBins(const Cmd &o);
        static void printCenters(const Cmd &o);

    protected:
        void findStart(std::unique_ptr<Walker>& w, double lb, double ub, UniformRNG& rng);

        double lnf_min;
        double flatness_criterion;

        int num_ranges;
        std::vector<std::vector<double>> bins;
};

#endif
