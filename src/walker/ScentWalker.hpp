#pragma once

#include <unordered_map>
#include <map>

#include "../Logging.hpp"
#include "../visualization/Svg.hpp"
#include "SpecWalker.hpp"

/** Agent based random walk on a hypercube.
 *
 *  * multiple, interacting walkers
 *  * random starting positions
 *  * all measurement methods, measure walker 0 by default
 *    (maybe with optional arguments, to access other walkers)
 *
 * The walkers leave a scent on their current site with some lifetime.
 * If another walker encounters a foreign scent, it will retreat, i.e.,
 * will in the next step step on a site without that scent.
 *
 * See also: https://doi.org/10.1371/journal.pcbi.1002008
 */
class ScentWalker final : public SpecWalker<int>
{
    public:
        ScentWalker(int d, int numSteps, int numWalker_in, int sideLength_in, int Tas_in, UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void updateSteps() final;
        void updatePoints(const int start) final;

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        virtual void svg(const std::string filename, const bool with_hull) const override;

        std::vector<std::vector<Step<int>>> pos;

        const int numWalker;
        const int sideLength;
        const int Tas;

    protected:
        std::vector<std::vector<Step<int>>> steps;
        std::vector<std::vector<Step<int>>> points;
        // hashmap: site -> deque[(time of last visit, who visited)]
        // std::unordered_map<Step<int>, std::deque<std::vector<std::pair<int, int>>>>;

    private:
        Step<int> newStep;
};
