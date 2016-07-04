#include "WangLandau.hpp"

WangLandau::WangLandau(const Cmd &o)
    : Simulation(o),
      lnf_min(o.lnf_min),
      flatness_criterion(o.flatness_criterion)
{
    num_ranges = o.wangLandauBorders.size() - 1;
    bins = generateBins(o);

    // write header to outfile
    oss << "# Two lines belong together.\n";
    oss << "# First lines are the centers of the bins.\n";
    oss << "# Second lines are unnormalized, log densities of the bin.\n";
    oss << "# Every pair is an independent Wang landau sampling (usable for error estimation).\n";
    oss << "# ranges: " << o.wangLandauBorders << "\n";
}

std::vector<std::vector<double>> WangLandau::generateBins(const Cmd &o)
{
    const int num_ranges = o.wangLandauBorders.size() - 1;
    std::vector<std::vector<double>> bins(num_ranges);
    for(int i=0; i<num_ranges; ++i)
    {
        const double lb = o.wangLandauBorders[i];
        const double ub = o.wangLandauBorders[i+1];
        const double binwidth = (ub - lb) / o.wangLandauBins;

        bins[i].reserve(o.wangLandauBins + 1 + o.wangLandauOverlap);

        // overlap to the left, but not for the leftmost
        int start = i>0 ? -o.wangLandauOverlap : 0;
        for(int j=start; j<o.wangLandauBins; ++j)
            bins[i].emplace_back(lb + j*binwidth);
        bins[i].emplace_back(ub);
    }

    return bins;
}

/// Create a starrting walk with lb < S < ub by a simple downhill strategy.
void WangLandau::findStart(std::unique_ptr<Walker>& w, double lb, double ub, UniformRNG& rng)
{
    do
    {
        // change one random number to another random number
        double oldS = S(w);
        w->change(rng);

        if((S(w) < lb && oldS > S(w)) || (S(w) > ub && oldS < S(w)))
            w->undoChange();

    } while(S(w) < lb || S(w) > ub);
}

void WangLandau::printCenters(const Cmd &o)
{
    for(auto i : generateBins(o))
    {
        Histogram g(i);
        std::cout << g.centers() << std::endl;
    }
}

/** Implementation of the Wang Landau algorithm.
 *
 * Literature used:
 *   * 10.1103/PhysRevLett.86.2050 (original paper)
 *   * 10.1103/PhysRevE.64.056101 (longer original paper)
 *   * http://cdn.intechopen.com/pdfs-wm/14019.pdf (implementations hints)
 *   * 10.1103/PhysRevE.67.067102 (what to do when encountering the boundary)
 */
void WangLandau::run()
{
    // dynamic because every iteration can take wildly different durations
    #pragma omp parallel for schedule(dynamic)
    for(int n=0; n<o.iterations; ++n)
    {
        // rngs should be local to the threads, with different seeds
        // FIXME: think about a better seed
        UniformRNG rngMC((o.seedMC+n) * (n+1));
        o.seedRealization += n;
        o.seedRealization *= n+1;

        std::unique_ptr<Walker> w;
        prepare(w, o);

        for(int i=0; i<num_ranges; ++i)
        {
            const double lb = bins[i].front();
            const double ub = bins[i].back();
            LOG(LOG_DEBUG) << "t" << omp_get_thread_num() << " [" << lb << ", " << ub << "] : [" << bins[i] << "]";

            Histogram H(bins[i]);
            Histogram g(bins[i]);

            findStart(w, lb, ub, rngMC);
            LOG(LOG_DEBUG) << "t" << omp_get_thread_num() << " " << lb << " < " << S(w) << " < " << ub << " start!";

            double lnf = 1;
            while(lnf > lnf_min)
            {
                LOG(LOG_DEBUG) << "t" << omp_get_thread_num() << " : ln f " << lnf;
                do
                {
                    double oldS = S(w);
                    w->change(rngMC);
                    ++tries;

                    double p_acc = exp(g[oldS] - g[S(w)]);
                    if(S(w) < lb || S(w) > ub || p_acc < rngMC())
                    {
                        w->undoChange();
                        ++fails;
                    }

                    g.add(S(w), lnf);
                    H.add(S(w));
                } while(H.min() < flatness_criterion * H.mean() || H.min() == 0);
                // run until the histogram is flat and we have a few samples
                H.reset();
                lnf /= 2;
            }
            // save g to file
            #pragma omp critical
            {
                oss << g.centers() << "\n";
                oss << g.get_data() << std::endl;
            }
        }
    }
}
