#include "FastWangLandau.hpp"

FastWangLandau::FastWangLandau(const Cmd &o)
    : WangLandau(o)
{
}

/** Implementation of the "Fast" 1/t Wang Landau algorithm extended by Entropic Sampling.
 *
 * Larger values of the final refinement parameter are ok, since
 * the simulation will be "corrected" by an entropic sampling
 * simulation after the Wang Landau estimation of g.
 *
 * Literature used:
 *   * 10.1103/PhysRevE.75.046701 (original paper)
 *   * 10.1063/1.2803061 (analytical)
 *   * http://arxiv.org/pdf/1107.2951v1.pdf (entropic sampling)
 */
void FastWangLandau::run()
{
    // parameters
    //~ const int initial_num_iterations = 1000;
    const int initial_num_iterations = 200;

    #pragma omp parallel for schedule(dynamic)
    for(int n=0; n<o.iterations; ++n)
    {
        // rngs should be local to the threads, with different seeds
        // FIXME: think about a better seed
        UniformRNG rngMC((o.seedMC+n) * (n+1));
        o.seedRealization = ((long long)(o.seedRealization + n) * (n+1)) % 1800000121;

        std::unique_ptr<Walker> w;
        prepare(w, o);

        for(int i=0; i<num_ranges; ++i)
        {
            const double lb = bins[i].front();
            const double ub = bins[i].back();
            LOG(LOG_DEBUG) << "[" << lb << ", " << ub << "] : [" << bins[i] << "]";

            Histogram H(bins[i]);
            Histogram g(bins[i]);

            int t = 0;
            double status = 1.;

            findStart(w, lb, ub, rngMC);
            LOG(LOG_DEBUG) << "found configuration: " << lb << " < " << S(w) << " < " << ub << " -> start!";
            LOG(LOG_DEBUG) << "begin phase 1 (exponential decrease)";
            // start first phase
            double lnf = 1;
            while(t < 10 || lnf > 1./t)
            {
                LOG(LOG_DEBUG) << "ln f = " << lnf << ", t = " << t;
                do
                {
                    for(int k=0; k < initial_num_iterations; ++k)
                    {
                        for(int j=0; j < o.steps; ++j)
                        {
                            double oldS = S(w);
                            w->change(rngMC);
                            ++tries;

                            double p_acc = std::exp(g[oldS] - g[S(w)]);
                            if(S(w) < lb || S(w) > ub || p_acc < rngMC())
                            {
                                w->undoChange();
                                ++fails;
                            }

                            g.add(S(w), lnf);
                            H.add(S(w));
                        }
                        ++t;
                    }
                } while(H.min() == 0);
                // run until we have one entry in each bin
                H.reset();
                lnf /= 2;
            }

            //start second phase
            status = 1./t;
            LOG(LOG_DEBUG) << "begin phase 2 (power-law decrease) at t=" << t;
            while(lnf > lnf_min)
            {
                lnf = 1./t;

                if(Logger::verbosity >= LOG_DEBUG && lnf < status)
                {
                    LOG(LOG_DEBUG) << "ln f = " << lnf << ", t = " << t;
                    status /= 2;
                }

                for(int j=0; j < o.steps; ++j)
                {
                    double oldS = S(w);
                    w->change(rngMC);
                    ++tries;

                    double p_acc = std::exp(g[oldS] - g[S(w)]);
                    if(S(w) < lb || S(w) > ub || p_acc < rngMC())
                    {
                        w->undoChange();
                        ++fails;
                    }

                    g.add(S(w), lnf);
                }
                ++t;
            }

            // perform entropic sampling with the bias g
            // this way the errors caused by too large f_final
            // are mitigated

            // the entropic sampling phase should be twice as long as
            // the previous phase
            LOG(LOG_DEBUG) << "begin phase 3 (entropic sampling) at t=" << t << " until t=" << 3*t;
            int t_limit = 2*t;
            for(int j=0; j<t_limit; ++j)
            {
                for(int k=0; k < o.steps; ++k)
                {
                    double oldS = S(w);
                    w->change(rngMC);
                    ++tries;

                    double p_acc = std::exp(g[oldS] - g[S(w)]);
                    if(S(w) < lb || S(w) > ub || p_acc < rngMC())
                    {
                        w->undoChange();
                        ++fails;
                    }
                    H.add(S(w));
                }
            }

            // remove the bias
            for(int j=0; j<g.get_num_bins(); ++j)
            {
                g[j] += H[j]/H.mean();
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
