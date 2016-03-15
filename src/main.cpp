#include <string>
#include <ctime>

#include <tclap/CmdLine.h>

#include "Walker.hpp"
#include "LoopErasedWalker.hpp"
#include "RNG.hpp"
#include "ConvexHull.hpp"

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
        TCLAP::ValueArg<int> verboseArg("v", "verbose", "verbosity level", false, 0, "integer");

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
        int v = verboseArg.getValue();

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
        std::cout << "Steps  " << w->nSteps() << "\n";
        std::cout << "Area   " << w->L() << "\n";
        std::cout << "Volume " << w->A() << "\n";
//        w->print();
        if(!svg_path.empty())
            w->svg(svg_path, true);

        if(v >= 1)
        {
            std::cout << "RNG: " << (double)(before_rng - before_walker) / CLOCKS_PER_SEC << " s\n";
            std::cout << "RW : " << (double)(before_walker - before_ch) / CLOCKS_PER_SEC << " s\n";
            std::cout << "CH : " << (double)(before_ch - before_output) / CLOCKS_PER_SEC << " s\n";
            std::cout << "OUT: " << (double)(before_output - clock()) / CLOCKS_PER_SEC << " s\n";
        }

        delete w;
    }
    catch(TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
}
