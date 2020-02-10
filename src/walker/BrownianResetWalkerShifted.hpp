#ifndef BROWNIANRESETWALKERSHIFTED_H
#define BROWNIANRESETWALKERSHIFTED_H

#include <list>
#include <iterator>
#include <unordered_set>

#include "../Logging.hpp"
#include "BrownianResetWalker.hpp"

/** Brownian Motion with Resetting
 *
 * A Brownian motion which resets with rate r but only after.the time `shift`
 */
class BrownianResetWalkerShifted final : public BrownianResetWalker
{
    public:
        BrownianResetWalkerShifted(int d, int numSteps, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);
        void updateSteps() final;

        double shift;
        void setP3(double shift) final;

    protected:
};

#endif
