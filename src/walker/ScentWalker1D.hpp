#ifndef SCENTWALKER1D_H
#define SCENTWALKER1D_H

#include <unordered_set>
#include <unordered_map>
#include <map>

#include "../Logging.hpp"
#include "../visualization/Svg.hpp"
#include "../visualization/GnuplotContour.hpp"
#include "../stat/HistogramND.hpp"
#include "SpecWalker.hpp"

/** tracks the scent marks on the line
 *
 * vector (space) -> vector (who) -> time
 *
 * the idea is that we just save for site i for each agent j the time it was
 * marked last: world[i][j] = t. Every time an agent enters a site we can
 * determine from the difference of current time, time stamp and active scent
 * time if this site is marked by an adversary.
 */
typedef std::vector<std::vector<int>> World;

/** Agent based random walk on a 1 dimensional line.
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
class ScentWalker1D final : public SpecWalker<int>
{
    public:
        ScentWalker1D(int d, int numSteps, int numWalker_in, int sideLength_in, int Tas_in, agent_start_t start_configuration, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false, bool save_histograms_in=false);

        void updatedNumWalker();
        void reconstruct() final;

        void updateSteps() final;
        void updateWorld(int site, int time);

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        // virtual void svg(const std::string filename, const bool with_hull=false) const override;
        // virtual void gp(const std::string filename, const bool with_hull=false) const override;
        // void svg_histogram(const std::string filename) const;

        int numWalker;          ///< total number of competing agents
        const int sideLength;   ///< side length of the world
        const int Tas;          ///< number of time steps the scentmarks persist
        const int relax;        ///< number of steps to relax the walk before measurements are taken
        bool periodic;          ///< use periodic or open (closed?) boundaries

        agent_start_t start_configuration;

        const bool save_histograms;   ///< save auxillary information for visualization

    protected:
        std::vector<Step<int>> starts;  ///< initial positions of walkers
        World initial_trail;            ///< initial trail configuration
        std::vector<HistogramND> histograms;

        Step<int> undo_start;

        std::vector<Step<int>> step, pos;
};

#endif
