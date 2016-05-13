#include "Cmd.hpp"

/// static verbosity level
int Logger::verbosity = 0;

/** Constructs the command line parser, given argc and argv.
 */
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
        TCLAP::ValueArg<int> t_eqArg("", "t_eq", "equilibration time to use", false, -1, "integer");
        TCLAP::ValueArg<int> t_eqMaxArg("", "t_eq_max", "maximum number equilibration time, abort simulation if not equilibrated ", false, 1e5, "integer");
        TCLAP::ValueArg<int> seedMCArg("x", "seedMC", "seed for Monte Carlo", false, 0, "integer");
        TCLAP::ValueArg<int> seedRArg("y", "seedR", "seed for realizations", false, 0, "integer");
        TCLAP::ValueArg<int> dimArg("d", "dimension", "dimension of the system", false, 2, "integer");
        TCLAP::ValueArg<int> parallelArg("", "parallel", "use openMP to use this many cpus, zero means all (only available for Wang Landau Sampling)", false, 0, "integer");
        TCLAP::ValueArg<double> thetaArg("T", "theta", "temperature for the large deviation scheme", false, 0, "double");
        TCLAP::ValueArg<std::string> tmpPathArg("", "tmp", "path for temporary files", false, ".", "string");
        TCLAP::ValueArg<std::string> dataPathArg("o", "output", "datafile for the output", false, "out.dat", "string");
        TCLAP::ValueArg<std::string> confPathArg("O", "confoutput", "datafile for the raw output", false, "", "string");
        TCLAP::ValueArg<std::string> svgArg("s", "svg", "svg filename, will be a xy projection", false, "", "string");
        TCLAP::ValueArg<std::string> povArg("p", "pov", "povray filename, will be a xyz projection", false, "", "string");
        TCLAP::ValueArg<std::string> gpArg("g", "gnuplot", "gnuplot filename, will be a xyz projection", false, "", "string");
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

        std::vector<int> wt({1, 2, 3, 4, 5, 6});
        TCLAP::ValuesConstraint<int> allowedWT(wt);
        TCLAP::ValueArg<int> typeArg("t", "type", "type of walk:\n"
                                                  "\tlattice random walk : 1 (default)\n"
                                                  "\tlooperased walk     : 2\n"
                                                  "\tself-avoiding walk  : 3\n"
                                                  "\treal random walk    : 4\n"
                                                  "\tGaussian random walk: 5\n"
                                                  "\tLevy flight         : 6",
                                     false, 1, &allowedWT);

        std::vector<int> ch({1, 2, 3, 4});
        TCLAP::ValuesConstraint<int> allowedCH(ch);
        TCLAP::ValueArg<int> chAlgArg("c", "convexHullAlgo", "convex hull algorithm:\n"
                                                             "\tquickhull (QHull)     : 1 (default)\n"
                                                             "\tAndrews Monotone Chain: 2\n"
                                                             "\tGraham Scan           : 3\n"
                                                             "\tJarvis March          : 4",
                                      false, 1, &allowedCH);
        std::vector<int> wo({1, 2});
        TCLAP::ValuesConstraint<int> allowedWO(wo);
        TCLAP::ValueArg<int> wantedobservableArg("w", "wantedObservable", "observable for which the probability density is desired:\n"
                                                                          "\tsurface area (L)    : 1 (default)\n"
                                                                          "\tvolume       (A)    : 2",
                                                 false, 1, &allowedWO);

        std::vector<int> sm({1, 2});
        TCLAP::ValuesConstraint<int> allowedSM(sm);
        TCLAP::ValueArg<int> samplingMethodArg("m", "samplingMethod", "Sampling Method to use:\n"
                                                                      "\tMetropolis          : 1 (default)\n"
                                                                      "\tWang Landau         : 2",
                                                 false, 1, &allowedSM);

        TCLAP::MultiArg<double> wangLandauBordersMArg("e", "energyBorder", "specifies inside which energy ranges, i.e., "
                                                                           "volumes or areas, the Wang Landau sampling "
                                                                           "should be run parallel. If this is smaller "
                                                                           "than the 'parallel' argument, not all cores "
                                                                           "will be used.", false, "double");
        TCLAP::ValueArg<int> wangLandauOverlapArg("E", "energyOverlap", "specifies how many bins between adjacent ranges "
                                                                        "should overlap. (default 10)",
                                                                        false, 10, "integer");
        TCLAP::ValueArg<int> wangLandauBinsArg("B", "energyBins", "specifies how many bins each range should have (default 100)",
                                                                      false, 100, "integer");

        // switch argument
        // -short, --long, description, default
        TCLAP::SwitchArg aklHeuristicSwitch("a", "aklHeuristic", "enables the Akl Toussaint heuristic", false);
        TCLAP::SwitchArg simpleSamplingSwitch("", "simplesampling", "use simple sampling instead of the large deviation scheme", false);
        TCLAP::SwitchArg benchmarkSwitch("b", "benchmark", "perform benchmark", false);
        TCLAP::SwitchArg quietSwitch("q", "quiet", "quiet mode (equal to -v 0)", false);

        // Add to the parser
        cmd.add(numArg);
        cmd.add(iterationsArg);
        cmd.add(t_eqArg);
        cmd.add(t_eqMaxArg);
        cmd.add(seedRArg);
        cmd.add(seedMCArg);
        cmd.add(dimArg);
        cmd.add(parallelArg);
        cmd.add(thetaArg);
        cmd.add(chAlgArg);
        cmd.add(wantedobservableArg);
        cmd.add(samplingMethodArg);
        cmd.add(wangLandauBordersMArg);
        cmd.add(wangLandauBinsArg);
        cmd.add(wangLandauOverlapArg);
        cmd.add(typeArg);
        cmd.add(svgArg);
        cmd.add(povArg);
        cmd.add(gpArg);
        cmd.add(dataPathArg);
        cmd.add(confPathArg);
        cmd.add(tmpPathArg);

        cmd.add(aklHeuristicSwitch);
        cmd.add(simpleSamplingSwitch);
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

        type = (walk_type_t) typeArg.getValue();
        LOG(LOG_INFO) << "Type                       " << TYPE_LABEL[type];

        steps = numArg.getValue();
        LOG(LOG_INFO) << "Number of steps            " << steps;

        d = dimArg.getValue();
        LOG(LOG_INFO) << "Dimension                  " << d;

        bool aklHeuristic = aklHeuristicSwitch.getValue();
        int tmp = chAlgArg.getValue();
        tmp = (tmp-1)*2 + 1;
        if(aklHeuristic)
            tmp++;
        chAlg = (hull_algorithm_t) tmp;
        LOG(LOG_INFO) << "Convex Hull Algorithm      " << CH_LABEL[chAlg];

        seedRealization = seedRArg.getValue();
        LOG(LOG_INFO) << "Seed for the realization   " << seedRealization;

        seedMC = seedMCArg.getValue();
        LOG(LOG_INFO) << "Seed for the MC simulation " << seedMC;

        wantedObservable = (wanted_observable_t) wantedobservableArg.getValue();
        LOG(LOG_INFO) << "Wanted Observable          " << WANTED_OBSERVABLE_LABEL[wantedObservable];

        svg_path = svgArg.getValue();
        sampling_method = (sampling_method_t) samplingMethodArg.getValue();
        LOG(LOG_INFO) << "Sampling Method            " << SAMPLING_METHOD_LABEL[sampling_method];

        iterations = iterationsArg.getValue();
        if(sampling_method == SM_WANG_LANDAU)
            iterations = 1;
        LOG(LOG_INFO) << "Number of MC iterations    " << iterations;

        simpleSampling = simpleSamplingSwitch.getValue();
        if(simpleSampling && sampling_method == SM_METROPOLIS)
        {
            LOG(LOG_INFO) << "simple sampling            ";
        }

        t_eq = t_eqArg.getValue();
        if(sampling_method == SM_METROPOLIS)
            if(t_eq >= 0)
            {
                LOG(LOG_INFO) << "Set equilibration time to  " << t_eq;
            }

        t_eqMax = t_eqMaxArg.getValue();
        if(sampling_method == SM_METROPOLIS)
        {
            LOG(LOG_INFO) << "Abort simulation if t_eq > " << t_eqMax;
        }

        theta = thetaArg.getValue();
        if(!simpleSampling && sampling_method == SM_METROPOLIS)
        {
            LOG(LOG_INFO) << "Theta                      " << theta;
        }

        wangLandauBorders = wangLandauBordersMArg.getValue();
        wangLandauBins = wangLandauBinsArg.getValue();
        wangLandauOverlap = wangLandauOverlapArg.getValue();
        if(sampling_method == SM_WANG_LANDAU)
        {
            if(wangLandauBorders.size() < 2)
            {
                LOG(LOG_ERROR) << "specify at least two borders: -e <lower> -e <upper>";
                exit(1);
            }
            std::sort(wangLandauBorders.begin(), wangLandauBorders.end());
            LOG(LOG_INFO) << "Borders of ranges for Wang Landau Sampling: \n                  " << wangLandauBorders;
            LOG(LOG_INFO) << "Bins each range:           " << wangLandauBins;
            LOG(LOG_INFO) << "Overlap between ranges:    " << wangLandauOverlap;
        }

        parallel = parallelArg.getValue();
        if(sampling_method == SM_WANG_LANDAU)
        {
            LOG(LOG_INFO) << "CPUs to use                " << (parallel ? std::to_string(parallel) : "all");
        }

        svg_path = svgArg.getValue();
        if(!svg_path.empty())
        {
            LOG(LOG_INFO) << "Path to store the SVG      " << svg_path;
        }

        pov_path = povArg.getValue();
        if(!pov_path.empty())
        {
            LOG(LOG_INFO) << "Path to store the povray   " << pov_path;
        }

        gp_path = gpArg.getValue();
        if(!gp_path.empty())
        {
            LOG(LOG_INFO) << "Path to store the gnuplot  " << gp_path;
        }

        data_path = dataPathArg.getValue();
        LOG(LOG_INFO) << "Path to store the data     " << data_path;

        conf_path = confPathArg.getValue();
        if(!conf_path.empty())
        {
            LOG(LOG_INFO) << "Path to store the config   " << conf_path;
        }

        tmp_path = tmpPathArg.getValue();
        LOG(LOG_INFO) << "Path for temporary files   " << tmp_path;
    }
    catch(TCLAP::ArgException &e)  // catch any exceptions
    {
        LOG(LOG_ERROR) << e.error() << " for arg " << e.argId();;
    }
}
