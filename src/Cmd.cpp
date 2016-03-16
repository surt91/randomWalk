#include "Cmd.hpp"

// static verbosity level
int Logger::verbosity = 0;

Cmd::Cmd(int argc, char** argv)
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
        TCLAP::ValueArg<int> verboseArg("v", "verbose", "verbosity level:\n"
                                                        "\tquiet  : 0\n"
                                                        "\talways : 1\n"
                                                        "\terror  : 2\n"
                                                        "\twarning: 3\n"
                                                        "\tinfo   : 4 (default)\n"
                                                        "\tdebug  : 5\n"
                                                        "\tdebug2 : 6\n"
                                                        "\tdebug3 : 7",
                                        false, 4, "integer");

        std::vector<int> allowedTypes_;
        allowedTypes_.push_back(1);
        allowedTypes_.push_back(2);
        TCLAP::ValuesConstraint<int> allowedTypes(allowedTypes_);
        TCLAP::ValueArg<int> typeArg("t", "type", "type of walk:\n"
                                                  "\trandom walk    : 1 (default)\n"
                                                  "\tlooperased walk: 2",
                                     false, 1, &allowedTypes);

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

        // Parse the argv array.
        cmd.parse(argc, argv);


        // Get the value parsed by each arg.
        n = numArg.getValue();
        seed = seedArg.getValue();
        svg_path = svgArg.getValue();
        d = dimArg.getValue();
        type = typeArg.getValue();
        Logger::verbosity = verboseArg.getValue();
    }
    catch(TCLAP::ArgException &e)  // catch any exceptions
    {
        std::stringstream ss;
        ss << e.error() << " for arg " << e.argId();
        log<LOG_ERROR>("") << ss.str();
    }
}
