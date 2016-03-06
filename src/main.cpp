#include <string>

#include <tclap/CmdLine.h>

#include "Walker.hpp"
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
        TCLAP::ValueArg<std::string> svgArg("s", "svg", "svg filename", false, "", "string");

        // switch argument
        // -short, --long, description, default
//        TCLAP::SwitchArg reverseSwitch("r","reverse","Print name backwards", false);

        // Add to the parser
        cmd.add(numArg);
        cmd.add(seedArg);
        cmd.add(svgArg);
//        cmd.add(reverseSwitch);
        // Parse the argv array.
        cmd.parse( argc, argv );

        // Get the value parsed by each arg.
        std::string svg_path = svgArg.getValue();
        int n = numArg.getValue();
        int seed = seedArg.getValue();

        std::vector<double> numbers = rng(n, seed);
        Walker w(2, numbers);
        auto ch = w.convexHull();
        ch.hullPoints();
        std::cout << "Aea    " << ch.L();
        std::cout << "Volume " << ch.A();
        w.print();
        if(!svg_path.empty())
            w.svg(svg_path, true);
    }
    catch(TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
}
