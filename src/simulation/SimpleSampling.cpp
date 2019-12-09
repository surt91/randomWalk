#include "SimpleSampling.hpp"

SimpleSampling::SimpleSampling(const Cmd &o)
    : Simulation(o)
{
}

void SimpleSampling::run()
{
    UniformRNG rngMC(o.seedMC);

    oss << "# simple sampling simulation with steps=" << o.steps << "\n";

    // header
    oss << "# sweeps L A";
    oss << " r r2 maxDiameter spanX SpanY numOnHull oblateness visitedSites length stepstaken argminX argmaxX minX maxX";
    for(auto i : o.passageTimeStarts)
        oss << " c" << i;
    for(auto i : o.passageTimeStarts)
        oss << " z" << i;
    oss << "\n";

    std::unique_ptr<Walker> w;
    prepare(w, o);

    for(int i=0; i<o.iterations; ++i)
    {
        w->generate_independent_sample();

        // save measurements to file
        if(!o.conf_path.empty())
            w->saveConfiguration(o.conf_path);

        LOG(LOG_DEBUG) << "Iteration: " << i;

        oss << i << " "
            << w->L() << " "
            << w->A() << " ";

        oss << w->r() << " "
            << w->r2() << " "
            << w->maxDiameter() << " "
            << w->rx() << " "
            << w->ry() << " "
            << w->num_on_hull() << " "
            << w->oblateness() << " "
            << w->visitedSites() << " "
            << w->enclosedSites() << " "
            << w->length() << " "
            << w->steps_taken() << " "
            << w->argminx() << " "
            << w->argmaxx() << " "
            << w->minx() << " "
            << w->maxx() << " ";

        if(auto r = dynamic_cast<ScentWalker*>(w.get()))
            oss << r->interactions();

        //~ auto c = w->correlation(o.passageTimeStarts);
        //~ for(auto j : c)
            //~ oss << j << " ";
        for(auto j : o.passageTimeStarts)
            oss << w->passage(j) << " ";

        // flush after every iteration
        oss << std::endl;

        checksum += S(w);
    }

    checksum /= o.iterations;

    // save visualizations
    if(!o.svg_path.empty())
        w->svg(o.svg_path, true);

    if(!o.pov_path.empty())
        w->pov(o.pov_path, true);

    if(!o.gp_path.empty())
        w->gp(o.gp_path, true);

    if(!o.threejs_path.empty())
        w->threejs(o.threejs_path, true);
}
