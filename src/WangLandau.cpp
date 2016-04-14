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
        w->degenerate();
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

void WangLandau::run()
{
    // 10.1103/PhysRevLett.86.2050 (original paper)
    // http://cdn.intechopen.com/pdfs-wm/14019.pdf (implementations hints)
    // parameters of the algorithm
    const double lnf_min = 1e-8;
    const double flatness_criterion = 0.8;

    // Histogram and DensityOfStates need the same binning ... probably
    const double lb = getLowerBound();
    const double ub = getUpperBound() + 1;

    // do not make a bin for every integer, since not every integer is possible
    const int bins = (ub-lb)/3;
    LOG(LOG_DEBUG) << lb << " " << ub << " " << bins;

    Histogram H(bins, lb, ub);
    DensityOfStates g(bins, lb, ub);

    oss << "# First line are the centers of the bins\n";
    oss << "# Following lines are unnormalized, log densities of the bin\n";
    oss << "# one line per iteration of the wang landau algorithm\n";
    oss << g.binCentersString() << std::endl;

    // run in parallel, in o.parallel threads, or all if not specified
    if(o.parallel)
        omp_set_num_threads(o.parallel);

    #pragma omp parallel firstprivate(H, g)
    {
        // rngs should be local to the threads, with different seeds
        // FIXME: think about a better seed
        int id = omp_get_thread_num();
        UniformRNG rngMC((o.seedMC+id) * (id+1));

        std::unique_ptr<Walker> w;
        prepare(w);

        // use dynamic schedule, since single iterations may need strongly fluctuating time
        #pragma omp for schedule(dynamic)
        for(int i=0; i<o.iterations; ++i)
        {
            double lnf = 1;
            while(lnf > lnf_min)
            {
                LOG(LOG_TOO_MUCH) << "t" << omp_get_thread_num() << " : ln f " << lnf;
                do
                {
                    double oldS = S(w);
                    w->change(rngMC);
                    ++trys;

                    double p_acc = exp(g[oldS] - g[S(w)]);
                    if(!g.checkBounds(S(w)) || p_acc < rngMC())
                    {
                        w->undoChange();
                        ++fail;
                    }

                    g[S(w)] += lnf;
                    H.add(S(w));
                } while(H.min() < flatness_criterion * H.mean());
                // run until the histogram is flat and we have a few samples
                H.reset();
                lnf /= 2;
            }
            // save g to file
            #pragma omp critical
            {
                oss << g.dataString() << std::endl;
            }

            g.reset();
        }
    }
}
