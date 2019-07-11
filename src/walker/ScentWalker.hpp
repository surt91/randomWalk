#ifndef SCENTWALKER_H
#define SCENTWALKER_H

#include <unordered_set>
#include <unordered_map>
#include <map>

#include "../Logging.hpp"
#include "../visualization/Svg.hpp"
#include "../visualization/GnuplotContour.hpp"
#include "../stat/HistogramND.hpp"
#include "SpecWalker.hpp"

/// tracks scent for one site
typedef std::map<int, int> Site;

/** tracks the scent marks on the plane
 *
 * hashmap: site -> (map: who -> when)
 */
typedef std::unordered_map<Step<int>, Site> Field;

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
 *
 * \image html scent.png "histogram where different agents spend time"
 */
class ScentWalker final : public SpecWalker<int>
{
    public:
        ScentWalker(int d, int numSteps, int numWalker_in, int sideLength_in, int Tas_in, agent_start_t start_configuration, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false, bool save_histograms_in=false);

        void updatedNumWalker();
        void reconstruct() final;

        void updateSteps() final;
        void updateField(Site &site, int time);

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        virtual void svg(const std::string filename, const bool with_hull=false) const override;
        virtual void gp(const std::string filename, const bool with_hull=false) const override;
        void svg_histogram(const std::string filename) const;

        /// counts the number of adversary walkers with which the walker of
        /// interest interacts (between `0` and `numWalker-1`).
        /// Will probably only be sensible for simple sampling
        int interactions(int walkerId=0);

        int numWalker;          ///< total number of competing agents
        const int sideLength;   ///< side length of the square world
        const int Tas;          ///< number of time steps the scentmarks persist
        const int relax;        ///< number of steps to relax the walk before measurements are taken
        bool periodic;          ///< use periodic or open (closed?) boundaries

        agent_start_t start_configuration;

        const bool save_histograms;   ///< save auxillary information for visualization

    protected:
        std::vector<Step<int>> starts;  ///< initial positions of walkers
        std::vector<HistogramND> histograms;

        Step<int> undo_start;

        /// records the walker ids, who interacted with any walker
        std::vector<std::unordered_set<int>> interaction_sets;

        std::vector<Step<int>> step, pos;
};

#endif
