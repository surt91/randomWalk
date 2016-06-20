#include "FastWangLandau.hpp"

FastWangLandau::FastWangLandau(const Cmd &o)
    : WangLandau(o)
{
}

/** Implementation of the "Fast" 1/t Wang Landau algorithm.
 *
 * Literature used:
 *   * 10.1103/PhysRevE.75.046701 (original paper)
 *   * 10.1063/1.2803061 (analytical)
 */
void FastWangLandau::run()
{
    // parameters
    const int initial_num_iterations = 1000;

    // run in parallel, in o.parallel threads, or all if not specified
    if(o.parallel)
    {
        omp_set_num_threads(o.parallel);
    }

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

            int t = 0;
            double status = 1.;

            findStart(w, lb, ub, rngMC);
            LOG(LOG_DEBUG) << "t" << omp_get_thread_num() << " " << lb << " < " << S(w) << " < " << ub << " start!";

            // start first phase
            double lnf = 1;
            while(t < 10 && lnf > 1./t)
            {
                LOG(LOG_DEBUG) << "t" << omp_get_thread_num() << " : ln f = " << lnf << ", t = " << t;
                do
                {
                    for(int i=0; i < initial_num_iterations; ++i)
                    {
                        double oldS = S(w);
                        w->change(rngMC);
                        ++t;

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
                // run until we have one entry in each bin
                H.reset();
                lnf /= 2;
            }

            //start second phase
            status = 1./t;
            LOG(LOG_DEBUG) << "t" << omp_get_thread_num() << " : begin phase 2 at t=" << t;
            while(lnf > lnf_min)
            {
                {
                    lnf = 1./t;

                    if(Logger::verbosity >= LOG_DEBUG)
                    {
                        if(lnf < status)
                        {
                            LOG(LOG_DEBUG) << "t" << omp_get_thread_num() << " : ln f = " << lnf << ", t = " << t;
                            status /= 2;
                        }
                    }

                    double oldS = S(w);
                    w->change(rngMC);
                    ++t;

                    double p_acc = exp(g[oldS] - g[S(w)]);
                    if(S(w) < lb || S(w) > ub || p_acc < rngMC())
                    {
                        w->undoChange();
                        ++fails;
                    }

                    g.add(S(w), lnf);
                }
            }

            // save g to file
            #pragma omp critical
            {
                oss << g.centers() << "\n";
                oss << g.get_data() << std::endl;
                tries += t;
            }
        }
    }
}
