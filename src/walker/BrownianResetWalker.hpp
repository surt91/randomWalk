#ifndef BROWNIANRESETWALKER_H
#define BROWNIANRESETWALKER_H

#include <list>
#include <iterator>
#include <unordered_set>

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** Brownian Motion with Resetting
 *
 * A Brownian motion which resets with rate r.
 *
 * See also:
 * doi: 10.1103/PhysRevLett.106.160601
 *
 * \image html BRRW.svg "example of a Brownian motion with resetting, \f$r = 0.1, T = 100, N = 10000\f$"
 */
class BrownianResetWalker : public SpecWalker<double>
{
    public:
        BrownianResetWalker(int d, int numSteps, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void reconstruct() final;

        void updateSteps() override;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        double resetrate;
        void setP1(double beta) final;
        double total_time;
        void setP2(double total_time) final;

        double argminx() const final;
        double argmaxx() const final;

        int num_resets() const final;
        int maxsteps_partialwalk() const final;
        double maxlen_partialwalk() const final;

        void svg(const std::string filename, const bool with_hull) const final;

    protected:
        Step<double> genStep(std::vector<double>::iterator first) const;

        std::vector<double> undo_values;

        std::vector<size_t> reset_times;

        double delta_t;
};

#endif
