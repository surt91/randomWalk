#pragma once

#include <unordered_map>
#include <map>

#include "../Logging.hpp"
#include "../visualization/Svg.hpp"
#include "../stat/HistogramND.hpp"
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

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        virtual void svg(const std::string filename, const bool with_hull) const override;
        void svg_histogram(const std::string filename) const;

        const int numWalker;
        const int sideLength;
        const int Tas;
        const int relax;    //< number of steps to relax the walk before measurements are taken

    protected:
        std::vector<Step<int>> starts;
        std::vector<std::vector<Step<int>>> pos;
        std::vector<Step<int>> tmp_steps;
        std::vector<HistogramND> histograms;

    private:
        Step<int> newStep;
        Step<int> undoStep;
};
