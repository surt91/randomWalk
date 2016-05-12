#include "WangLandau.hpp"

WangLandau::WangLandau(const Cmd &o)
    : Simulation(o)
{
}

double WangLandau::getUpperBound()
{
    double S_min = 0;

    std::unique_ptr<Walker> w;
    prepare(w);

    // the degenerate case is -- hopefully -- the case of maximum Volume
    if(o.wantedObservable == WO_VOLUME)
    {
        w->degenerateMaxVolume();
        S_min = S(w);
    }
    else
    {
        w->degenerateMaxSurface();
        S_min = S(w);
    }

    return S_min;
}

double WangLandau::getLowerBound()
{
    if(o.wantedObservable == WO_VOLUME)
        return 0;

    double S_min = 0;
    if(o.type == WT_RANDOM_WALK)
        S_min = 2;
    else
        S_min = 4*sqrt(o.steps);

    return S_min;
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

void WangLandau::run()
{
    // 10.1103/PhysRevLett.86.2050 (original paper)
    // http://cdn.intechopen.com/pdfs-wm/14019.pdf (implementations hints)
    // parameters of the algorithm
    const double lnf_min = 1e-8;
    const double flatness_criterion = 0.8;

    const int num_ranges = o.wangLandauBorders.size() - 1;

    std::vector<double> bins;
    bins.reserve(num_ranges * o.wangLandauBins);
    for(int i=0; i<num_ranges; ++i)
        for(int j=0; j<num_ranges; ++j)
        {
            bins.emplace_back();
        }

    oss.precision(12);
    oss << "# Two lines belong together.\n";
    oss << "# First lines are the centers of the bins.\n";
    oss << "# Second lines are unnormalized, log densities of the bin.\n";
    oss << "# One pair per energy range of the wang landau algorithm.\n";
    oss << "# ranges: " << o.wangLandauBorders << "\n";

    // run in parallel, in o.parallel threads, or all if not specified
    if(o.parallel)
        omp_set_num_threads(o.parallel);

    #pragma omp parallel for schedule(dynamic)
    for(int i=0; i<num_ranges; ++i)
    {
        // Histogram and DensityOfStates need the same binning
        // FIXME: Histograms of adjacent energy regions should overlap a bit
        const double lb = o.wangLandauBorders[i];
        const double ub = o.wangLandauBorders[i+1];
        // do not make a bin for every integer, since not every integer is possible
        const int bins = (ub-lb)/3;

        LOG(LOG_DEBUG) << "t" << omp_get_thread_num() << " " << lb << " " << ub << " " << bins;

        Histogram H(bins, lb, ub);
        Histogram g(bins, lb, ub);

        // rngs should be local to the threads, with different seeds
        // FIXME: think about a better seed
        int id = omp_get_thread_num();
        UniformRNG rngMC((o.seedMC+id) * (id+1));

        std::unique_ptr<Walker> w;
        prepare(w);

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
