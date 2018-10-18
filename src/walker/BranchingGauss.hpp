#ifndef BRANCHINGGAUSS_H
#define BRANCHINGGAUSS_H

#include <set>

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** A branching Gaussian walk
 */
class BranchingGauss final : public SpecWalker<double>
{
    public:
        BranchingGauss(int d, int numSteps, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void reconstruct() final;

        void updateSteps() final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        void changeSingle(UniformRNG &rng);
        void undoChangeSingle();

        void degenerateMaxVolume();
        void svg(const std::string filename, const bool with_hull=false) const final;

    protected:
        Step<double> genStep(std::vector<double>::iterator first) const;

        const double branch_prob;
        const double perish_prob;
        std::vector<std::vector<Step<double>>> branches;

        std::vector<double> undo_values;
};

#endif
