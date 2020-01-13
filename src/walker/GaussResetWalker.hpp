#ifndef GAUSSRESETWALKER_H
#define GAUSSRESETWALKER_H

#include <list>
#include <iterator>
#include <unordered_set>

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** Gaussian Resetting Random Walk
 *
 * A Walk which resets with probability p.
 *
 * See also:
 * doi: 10.1103/PhysRevLett.106.160601
 *
 * \image html GRRW.svg "example of a gaussian resetting random walk, \f$p = 0.1, T = 100\f$"
 */
class GaussResetWalker final : public SpecWalker<double>
{
    public:
        GaussResetWalker(int d, int numSteps, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void reconstruct() final;

        void updateSteps() final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        double resetrate;
        void setP1(double beta) final;

        int num_resets() const final;
        int maxsteps_partialwalk() const final;
        double maxlen_partialwalk() const final;

    protected:
        Step<double> genStep(std::vector<double>::iterator first) const;

        std::vector<double> undo_values;
};

#endif
