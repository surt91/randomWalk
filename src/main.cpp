#include <string>
#include <ctime>

#include <tclap/CmdLine.h>

#include "Logging.hpp"
#include "Walker.hpp"
#include "LoopErasedWalker.hpp"
#include "RNG.hpp"
#include "ConvexHull.hpp"
#include "misc.hpp"

int main(int argc, char** argv)
{
    // TCLAP throws exceptions
    try{
        // Usage, delimiter, version
        TCLAP::CmdLine cmd("Calculates random walks", ' ', "0.1");

        // value argument
        // -short, --long, description, required, default, type
        TCLAP::ValueArg<int> numArg("n", "number", "how many steps", true, 100, "integer");
        TCLAP::ValueArg<int> seedArg("x", "seed", "seed for rng", false, 0, "integer");
        TCLAP::ValueArg<std::string> svgArg("s", "svg", "svg filename, will be a xy projection", false, "", "string");
        TCLAP::ValueArg<int> dimArg("d", "dimension", "dimension of the system", false, 2, "integer");
        TCLAP::ValueArg<int> verboseArg("v", "verbose", "verbosity level", false, 4, "integer");

        std::vector<int> allowedTypes_;
        allowedTypes_.push_back(1);
        allowedTypes_.push_back(2);
        TCLAP::ValuesConstraint<int> allowedTypes(allowedTypes_);
        TCLAP::ValueArg<int> typeArg("t", "type", "type of walk:\n"
                                                          "\trandom walk    : 1 (default)\n"
                                                          "\tlooperased walk: 2", false, 1, &allowedTypes);

        // switch argument
        // -short, --long, description, default
//        TCLAP::SwitchArg reverseSwitch("r","reverse","Print name backwards", false);

        // Add to the parser
        cmd.add(numArg);
        cmd.add(seedArg);
        cmd.add(svgArg);
        cmd.add(dimArg);
        cmd.add(typeArg);
        cmd.add(verboseArg);
//        cmd.add(reverseSwitch);
        // Parse the argv array.
        cmd.parse(argc, argv);

        // Get the value parsed by each arg.
        std::string svg_path = svgArg.getValue();
        int n = numArg.getValue();
        int seed = seedArg.getValue();
        int type = typeArg.getValue();
        int d = dimArg.getValue();
        VERBOSITY_LEVEL = verboseArg.getValue();

        clock_t before_rng = clock();
        std::vector<double> numbers = rng(n, seed);

        clock_t before_walker = clock();
        Walker *w = NULL;
        if(type == 1)
            w = new Walker(d, numbers);
        else if(type == 2)
            w = new LoopErasedWalker(d, numbers);
        w->steps();

        clock_t before_ch = clock();
        w->convexHull();

        clock_t before_output = clock();
        w->hullPoints();
        log<LOG_INFO>("Steps :") << w->nSteps();
        log<LOG_INFO>("Area  :") << w->L();
        log<LOG_INFO>("Volume:") << w->A();
        log<LOG_TOO_MUCH>("Trajectory:") << w->print();
        if(!svg_path.empty())
            w->svg(svg_path, true);

        log<LOG_TIMING>("RNG: ") << time_diff(before_walker, before_rng);
        log<LOG_TIMING>("RW : ") << time_diff(before_ch, before_walker);
        log<LOG_TIMING>("CH : ") << time_diff(before_output, before_ch);
        log<LOG_TIMING>("OUT: ") << time_diff(clock(), before_output);

        delete w;
    }
    catch(TCLAP::ArgException &e)  // catch any exceptions
    {
//        log<LOG_ERROR>("") << e.error() << " for arg " << e.argId() << std::endl;
    }
}
