#include "LargeDeviation.hpp"

void run(const Cmd &o)
{
    UniformRNG rngReal(o.seedRealization);
    UniformRNG rngMC(o.seedMC);
    std::ofstream oss(o.data_path, std::ofstream::out);
    if(!oss.good())
    {
        log<LOG_ERROR>("Path is not writable") << o.data_path;
        throw std::invalid_argument("Path is not writable");
    }
    oss << "# large deviation simulation at theta=" << o.theta << " and steps=" << o.steps << "\n";
    oss << "# sweeps L A\n";

    // does not work for loop erased yet
    Walker w(o.d, o.steps, rngReal, o.chAlg);
    w.convexHull();

    for(int i=0; i<o.iterations; ++i)
    {
        // one sweep, i.e., one change try for each site
        for(int j=0; j<o.steps; ++j)
        {
            int rn_to_change = o.steps * rngMC();

            // change one random number to another random number
            // save the random number before the change
            double oldS = w.A();
            double oldRN = w.rnChange(rn_to_change, rngMC());

            // Metropolis rejection
            double p_acc = std::min({1.0, exp(-(w.A() - oldS)/o.theta)});
            log<LOG_TOO_MUCH>("newA") << w.A();
            log<LOG_TOO_MUCH>("oldA") << oldS;
            log<LOG_TOO_MUCH>("delta") << (w.A() - oldS);
            log<LOG_TOO_MUCH>("p_acc") << p_acc;
            if(p_acc < 1 - rngMC())
                w.rnChange(rn_to_change, oldRN);

        }
        log<LOG_TOO_MUCH>("Area  :") << w.L();
        log<LOG_TOO_MUCH>("Volume:") << w.A();
        log<LOG_DEBUG>("Iteration:") << i;
        oss << i << " " << w.L() << " " << w.A() << std::endl;
    }

    if(!o.svg_path.empty())
        w.svg(o.svg_path, true);
}
