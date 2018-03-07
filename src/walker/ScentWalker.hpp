#ifndef SCENTWALKER_H
#define SCENTWALKER_H

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
        ScentWalker(int d, int numSteps, int numWalker_in, int sideLength_in, int Tas_in, const UniformRNG &rng, hull_algorithm_t hull_algo, bool amnesia=false);

        void reconstruct() final;

        void updateSteps() final;
        void updateField(Site &site, int time);

        void change(UniformRNG &rng, bool update=true) final;
        void undoChange() final;

        virtual void svg(const std::string filename, const bool with_hull=false) const override;
        virtual void gp(const std::string filename, const bool with_hull=false) const override;
        void svg_histogram(const std::string filename) const;

        const int numWalker;
        const int sideLength;
        const int Tas;
        const int relax;    ///< number of steps to relax the walk before measurements are taken
        bool periodic;      ///< use periodic or open (closed?) boundaries

    protected:
        std::vector<Step<int>> starts;
        std::vector<HistogramND> histograms;

        std::vector<Step<int>> step, pos;

    private:
        Step<int> newStep;
        Step<int> undoStep;
};

#endif
