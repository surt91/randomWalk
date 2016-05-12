#include "FastWangLandau.hpp"

FastWangLandau::FastWangLandau(const Cmd &o)
    : WangLandau(o)
{
}

void FastWangLandau::run()
{
    // 10.1103/PhysRevE.75.046701
    // parameters of the algorithm
    const double lnf_min = 1e-8;

    // Histogram and DensityOfStates need the same binning ... probably
    double lb = getLowerBound();
    double ub = getUpperBound() + 1;

    // do not make a bin for every integer, since not every integer is possible
    const int bins = (ub-lb)/3;
    LOG(LOG_DEBUG) << lb << " " << ub << " " << bins;

    Histogram H(bins, lb, ub);
    Histogram g(bins, lb, ub);

    oss << "# First line are the centers of the bins\n";
    oss << "# Following lines are unnormalized, log densities of the bin\n";
    oss << "# one line per iteration of the wang landau algorithm\n";
    oss << g.centers() << std::endl;

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
            int t = 0;
            bool secondPhase = false;
            while(lnf > lnf_min)
            {
                //~ LOG(LOG_TOO_MUCH) << "t" << omp_get_thread_num() << " : ln f " << lnf;
                if(!secondPhase)
                {
                    do
                    {
                        for(int j=0; j < o.steps*1000; ++j)
                        {
                            ++t;
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
                        }

                    } while(H.min() == 0);
                    H.reset();
                    lnf /= 2;
                    if(lnf <= 1/t)
                    {
                        secondPhase = true;
                        LOG(LOG_DEBUG) << "t" << omp_get_thread_num() << " : begin phase 2 at " << t;
                    }
                }
                else
                {
                    lnf = 1/t;
                    double oldS = S(w);
                    w->change(rngMC);
                    double p_acc = exp(g[oldS] - g[S(w)]);
                    if(S(w) < lb || S(w) > ub || p_acc < rngMC())
                        w->undoChange();

                    g.add(S(w), lnf);
                }
            }
            // save g to file
            #pragma omp critical
            {
                oss << g.get_data() << std::endl;
            }

            g.reset();
        }
    }
}
