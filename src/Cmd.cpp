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
        TCLAP::ValueArg<int> numArg("N", "steps", "how many steps", false, 100, "integer");
        TCLAP::ValueArg<int> iterationsArg("n", "iterations", "how many MC tries", false, 100, "integer");
        TCLAP::ValueArg<int> seedMCArg("x", "seedMC", "seed for Monte Carlo", false, 0, "integer");
        TCLAP::ValueArg<int> seedRArg("y", "seedR", "seed for realizations", false, 0, "integer");
        TCLAP::ValueArg<int> dimArg("d", "dimension", "dimension of the system", false, 2, "integer");
        TCLAP::ValueArg<int> thetaArg("T", "theta", "temperature for the large deviation scheme", false, 2, "integer");
        TCLAP::ValueArg<std::string> dataPathArg("o", "output", "datafile for the output", false, "out.dat", "string");
        TCLAP::ValueArg<std::string> svgArg("s", "svg", "svg filename, will be a xy projection", false, "", "string");
        TCLAP::ValueArg<std::string> povArg("p", "pov", "povray filename, will be a xyz projection", false, "", "string");
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

        std::vector<int> allowedCH_;
        allowedCH_.push_back(1);
        allowedCH_.push_back(2);
        allowedCH_.push_back(3);
        allowedCH_.push_back(4);
        TCLAP::ValuesConstraint<int> allowedCH(allowedCH_);
        TCLAP::ValueArg<int> chAlgArg("c", "convexHullAlgo", "convex hull algorithm:\n"
                                                             "\tquickhull (QHull)     : 1 (default)\n"
                                                             "\tAndrews Monotone Chain: 2\n"
                                                             "\tGraham Scan           : 3\n"
                                                             "\tJarvis March          : 4",
                                      false, 1, &allowedCH);

        // switch argument
        // -short, --long, description, default
        TCLAP::SwitchArg aklHeuristicSwitch("a","aklHeuristic","enables the Akl Toussaint heuristic", false);
        TCLAP::SwitchArg benchmarkSwitch("b","benchmark","perform benchmark", false);

        // Add to the parser
        cmd.add(numArg);
        cmd.add(iterationsArg);
        cmd.add(seedRArg);
        cmd.add(seedMCArg);
        cmd.add(dimArg);
        cmd.add(thetaArg);
        cmd.add(chAlgArg);
        cmd.add(typeArg);
        cmd.add(verboseArg);
        cmd.add(svgArg);
        cmd.add(povArg);
        cmd.add(dataPathArg);

        cmd.add(aklHeuristicSwitch);
        cmd.add(benchmarkSwitch);

        // Parse the argv array.
        cmd.parse(argc, argv);


        Logger::verbosity = 4;
        benchmark = benchmarkSwitch.getValue();
        if(benchmark)
        {
            Logger(LOG_INFO) << "Benchmark Mode";
            return;
        }

        // Get the value parsed by each arg.
        Logger::verbosity = verboseArg.getValue();
        Logger(LOG_INFO) << "Verbosity                 " << Logger::verbosity;

        bool aklHeuristic = aklHeuristicSwitch.getValue();
        int tmp = chAlgArg.getValue();
        tmp = (tmp-1)*2 + 1;
        if(aklHeuristic)
            tmp++;
        chAlg = (hull_algorithm_t) tmp;
        Logger(LOG_INFO) << "Convex Hull Algorithm     " << CH_LABEL[chAlg];

        steps = numArg.getValue();
        Logger(LOG_INFO) << "Number of steps           " << steps;
        iterations = iterationsArg.getValue();
        Logger(LOG_INFO) << "Number of MC iterations   " << iterations;
        seedRealization = seedRArg.getValue();
        Logger(LOG_INFO) << "Seed for the realization  " << seedRealization;
        seedMC = seedMCArg.getValue();
        Logger(LOG_INFO) << "Seed for the MC simulation" << seedMC;
        d = dimArg.getValue();
        Logger(LOG_INFO) << "Dimension                 " << d;
        theta = thetaArg.getValue();
        Logger(LOG_INFO) << "Theta                     " << theta;
        type = typeArg.getValue();
        Logger(LOG_INFO) << "Type                      " << TYPE_LABEL[type];
        svg_path = svgArg.getValue();
        Logger(LOG_INFO) << "Path to store the SVG     " << svg_path;
        pov_path = povArg.getValue();
        Logger(LOG_INFO) << "Path to store the povray  " << pov_path;
        data_path = dataPathArg.getValue();
        Logger(LOG_INFO) << "Path to store the data    " << data_path;
    }
    catch(TCLAP::ArgException &e)  // catch any exceptions
    {
        Logger(LOG_ERROR) << e.error() << " for arg " << e.argId();;
    }
}
