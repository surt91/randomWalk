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
        TCLAP::ValueArg<double> thetaArg("T", "theta", "temperature for the large deviation scheme", false, 2, "double");
        TCLAP::ValueArg<std::string> tmpPathArg("", "tmp", "path for temporary files", false, ".", "string");
        TCLAP::ValueArg<std::string> dataPathArg("o", "output", "datafile for the output", false, "out.dat", "string");
        TCLAP::ValueArg<std::string> confPathArg("O", "confoutput", "datafile for the raw output", false, "out.conf", "string");
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
        allowedTypes_.push_back(3);
        TCLAP::ValuesConstraint<int> allowedTypes(allowedTypes_);
        TCLAP::ValueArg<int> typeArg("t", "type", "type of walk:\n"
                                                  "\trandom walk       : 1 (default)\n"
                                                  "\tlooperased walk   : 2\n"
                                                  "\tself-avoiding walk: 3",
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

        std::vector<int> allowedWO_;
        allowedWO_.push_back(1);
        allowedWO_.push_back(2);
        TCLAP::ValuesConstraint<int> allowedWO(allowedWO_);
        TCLAP::ValueArg<int> wantedobservableArg("w", "wantedObservable", "observable for which the probability density is desired:\n"
                                                                          "\tsurface area (L)    : 1 (default)\n"
                                                                          "\tvolume       (A)    : 2",
                                                 false, 1, &allowedWO);

        // switch argument
        // -short, --long, description, default
        TCLAP::SwitchArg aklHeuristicSwitch("a", "aklHeuristic", "enables the Akl Toussaint heuristic", false);
        TCLAP::SwitchArg benchmarkSwitch("b", "benchmark", "perform benchmark", false);
        TCLAP::SwitchArg quietSwitch("q", "quiet", "quiet mode (equal to -v 0)", false);

        // Add to the parser
        cmd.add(numArg);
        cmd.add(iterationsArg);
        cmd.add(seedRArg);
        cmd.add(seedMCArg);
        cmd.add(dimArg);
        cmd.add(thetaArg);
        cmd.add(chAlgArg);
        cmd.add(wantedobservableArg);
        cmd.add(typeArg);
        cmd.add(svgArg);
        cmd.add(povArg);
        cmd.add(dataPathArg);
        cmd.add(confPathArg);
        cmd.add(tmpPathArg);

        cmd.add(aklHeuristicSwitch);
        cmd.add(benchmarkSwitch);

        cmd.add(quietSwitch);
        cmd.add(verboseArg);

        // Parse the argv array.
        cmd.parse(argc, argv);


        Logger::verbosity = 4;
        benchmark = benchmarkSwitch.getValue();
        if(benchmark)
        {
            LOG(LOG_INFO) << "Benchmark Mode";
            return;
        }

        // Get the value parsed by each arg.
        if(quietSwitch.getValue())
            Logger::verbosity = 0;
        else
            Logger::verbosity = verboseArg.getValue();
        LOG(LOG_INFO) << "Verbosity                  " << Logger::verbosity;

        bool aklHeuristic = aklHeuristicSwitch.getValue();
        int tmp = chAlgArg.getValue();
        tmp = (tmp-1)*2 + 1;
        if(aklHeuristic)
            tmp++;
        chAlg = (hull_algorithm_t) tmp;
        LOG(LOG_INFO) << "Convex Hull Algorithm      " << CH_LABEL[chAlg];

        steps = numArg.getValue();
        LOG(LOG_INFO) << "Number of steps            " << steps;
        iterations = iterationsArg.getValue();
        LOG(LOG_INFO) << "Number of MC iterations    " << iterations;
        seedRealization = seedRArg.getValue();
        LOG(LOG_INFO) << "Seed for the realization   " << seedRealization;
        seedMC = seedMCArg.getValue();
        LOG(LOG_INFO) << "Seed for the MC simulation " << seedMC;
        d = dimArg.getValue();
        LOG(LOG_INFO) << "Dimension                  " << d;
        theta = thetaArg.getValue();
        LOG(LOG_INFO) << "Theta                      " << theta;
        wantedObservable = (wanted_observable_t) wantedobservableArg.getValue();
        LOG(LOG_INFO) << "Wanted Observable          " << WANTED_OBSERVABLE_LABEL[wantedObservable];
        type = (walk_type_t) typeArg.getValue();
        LOG(LOG_INFO) << "Type                       " << TYPE_LABEL[type];
        svg_path = svgArg.getValue();
        LOG(LOG_INFO) << "Path to store the SVG      " << svg_path;
        pov_path = povArg.getValue();
        LOG(LOG_INFO) << "Path to store the povray   " << pov_path;
        data_path = dataPathArg.getValue();
        LOG(LOG_INFO) << "Path to store the data     " << data_path;
        conf_path = confPathArg.getValue();
        LOG(LOG_INFO) << "Path to store the raw data " << conf_path;
        tmp_path = tmpPathArg.getValue();
        LOG(LOG_INFO) << "Path for temporary files   " << tmp_path;
    }
    catch(TCLAP::ArgException &e)  // catch any exceptions
    {
        LOG(LOG_ERROR) << e.error() << " for arg " << e.argId();;
    }
}
