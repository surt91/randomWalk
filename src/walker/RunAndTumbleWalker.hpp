#ifndef RUNANDTUMBLEWALKER_H
#define RUNANDTUMBLEWALKER_H

#include "../Logging.hpp"
#include "SpecWalker.hpp"

/** With probability 1-gamma, the walk does not change direction.
 *
 * \image html RTP.svg "example of a run-and-tumble particle"
 */
class RunAndTumbleWalker final : public SpecWalker<double>
{
    public:
        RunAndTumbleWalker(int d, int numSteps, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void reconstruct() final;

        void updateSteps() final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        void changeSingle(UniformRNG &rng);
        void undoChangeSingle();

        void setP1(double gamma) final;

    protected:
        Step<double> genStep(int idx) const;

        std::vector<double> undo_values;
        double undo_tumble;
        std::vector<double> random_tumble;

        double gamma;
};

#endif
