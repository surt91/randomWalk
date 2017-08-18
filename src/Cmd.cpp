#include "Cmd.hpp"

/** Constructs the command line parser, given argc and argv.
 */
Cmd::Cmd(int argc, char** argv)
{
    for(int i=0; i<argc; ++i)
    {
        text += argv[i];
        text += " ";
    }

    std::string version = VERSION " (Compiled: " __DATE__ " " __TIME__ ","
                                     "D_MAX=" + std::to_string(D_MAX) + ")";

    // TCLAP throws exceptions
    try{
        // Usage, delimiter, version
        TCLAP::CmdLine cmd("Calculates random walks", ' ', version);

        // value argument
        // -short, --long, description, required, default, type
        TCLAP::ValueArg<int> numArg("N", "steps", "how many steps", false, 100, "integer");
        TCLAP::ValueArg<int> numWalkerArg("M", "walker", "how many walker", false, 1, "integer");
        TCLAP::ValueArg<int> sweepArg("k", "sweep", "how many MC tries per sweep (default: number of steps)", false, -1, "integer");
        TCLAP::ValueArg<int> iterationsArg("n", "iterations", "how many MC sweeps", false, 100, "integer");
        TCLAP::ValueArg<int> t_eqArg("", "t_eq", "equilibration time to use", false, -1, "integer");
        TCLAP::ValueArg<int> t_eqMaxArg("", "t_eq_max", "maximum number equilibration time, abort simulation if not equilibrated ", false, 1e5, "integer");
        TCLAP::ValueArg<int> seedMCArg("x", "seedMC", "seed for Monte Carlo", false, 0, "integer");
        TCLAP::ValueArg<int> seedRArg("y", "seedR", "seed for realizations", false, 0, "integer");
        TCLAP::ValueArg<int> dimArg("d", "dimension", "dimension of the system", false, 2, "integer");
        TCLAP::MultiArg<int> passageTimeStartArg("z", "passageTimeStart", "reference point to start from", false, "integer");
        TCLAP::ValueArg<int> parallelArg("P", "parallel", "use openMP to use this many cpus, zero means all (only available for Wang Landau Sampling)", false, 1, "integer");
        TCLAP::MultiArg<double> thetaArg("T", "theta", "temperature for the large deviation scheme, multiple for Parallel Tempering", false, "double");
        TCLAP::ValueArg<double> muArg("", "mu", "mu of the Gaussian distribution, i.e., introducing a direction bias (only for t=7: correlated walk)", false, 0.0, "double");
        TCLAP::ValueArg<double> sigmaArg("", "sigma", "sigma of the Gaussian distribution, i.e., how narrow should the angle delta be (only for t=7: correlated walk)", false, 1.0, "double");
        TCLAP::ValueArg<int> widthArg("", "width", "width of the field (only for t=9: scent walk)", false, 10, "integer");
        TCLAP::ValueArg<int> tasArg("", "tas", "lifetime of the scent (only for t=9: scent walk)", false, 1000, "integer");
        TCLAP::ValueArg<double> lnfArg("", "lnf", "minimum value of ln(f) for the Wang Landau algorithm (default 1e-8)", false, 1e-8, "double");
        TCLAP::ValueArg<double> flatnessArg("", "flatness", "flatness criterion for the Wang Landau algorithm (default 0.8)", false, 0.8, "double");
        TCLAP::ValueArg<std::string> tmpPathArg("", "tmp", "path for temporary files", false, ".", "string");
        TCLAP::MultiArg<std::string> dataPathArg("o", "output", "datafile for the output, (for each -T / --theta one)", false, "string");
        TCLAP::MultiArg<std::string> confPathArg("O", "confoutput", "datafile for the raw output, (for each -T / --theta one)", false, "string");
        TCLAP::ValueArg<std::string> svgArg("s", "svg", "svg filename, will be a xy projection", false, "", "string");
        TCLAP::ValueArg<std::string> povArg("p", "pov", "povray filename, will be a xyz projection", false, "", "string");
        TCLAP::ValueArg<std::string> gpArg("g", "gnuplot", "gnuplot filename, will be a xyz projection", false, "", "string");
        TCLAP::ValueArg<std::string> threejsArg("j", "threejs", "three.js filename, will be a xyz projection", false, "", "string");
        TCLAP::ValueArg<std::string> logfileArg("L", "logfile", "log to file", false, "", "string");
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

        std::vector<int> wt({1, 2, 3, 4, 5, 6, 7, 8, 9});
        TCLAP::ValuesConstraint<int> allowedWT(wt);
        TCLAP::ValueArg<int> typeArg("t", "type", "type of walk:\n"
                                                  "\tlattice random walk   : 1 (default)\n"
                                                  "\tlooperased walk       : 2\n"
                                                  "\tself-avoiding walk    : 3\n"
                                                  "\treal random walk      : 4\n"
                                                  "\tGaussian random walk  : 5\n"
                                                  "\tLevy flight           : 6\n"
                                                  "\tCorrelated random walk: 7\n"
                                                  "\tEscape random walk    : 8\n"
                                                  "\tScent random walk     : 9\n",
                                     false, 1, &allowedWT);

        std::vector<int> ch({0, 1, 2, 3, 4});
        TCLAP::ValuesConstraint<int> allowedCH(ch);
        TCLAP::ValueArg<int> chAlgArg("c", "convexHullAlgo", "convex hull algorithm:\n"
                                                             "\tNOP (do nothing)      : 0\n"
                                                             "\tquickhull (QHull)     : 1 (default)\n"
                                                             "\tAndrews Monotone Chain: 2\n"
                                                             "\tGraham Scan           : 3\n"
                                                             "\tJarvis March          : 4",
                                      false, 1, &allowedCH);
        std::vector<int> wo({1, 2, 3});
        TCLAP::ValuesConstraint<int> allowedWO(wo);
        TCLAP::ValueArg<int> wantedobservableArg("w", "wantedObservable", "observable for which the probability density is desired:\n"
                                                                          "\tsurface area (L)    : 1 (default)\n"
                                                                          "\tvolume       (A)    : 2\n"
                                                                          "\tpassage time (t)    : 3",
                                                 false, 1, &allowedWO);

        std::vector<int> sm({0, 1, 2, 3, 4});
        TCLAP::ValuesConstraint<int> allowedSM(sm);
        TCLAP::ValueArg<int> samplingMethodArg("m", "samplingMethod", "Sampling Method to use:\n"
                                                                      "\tSimple Sampling     : 0\n"
                                                                      "\tMetropolis          : 1 (default)\n"
                                                                      "\tWang Landau         : 2\n"
                                                                      "\tFast Wang Landau    : 3\n"
                                                                      "\tParallel Tempering  : 4",
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
        TCLAP::SwitchArg onlyBoundsSwitch("", "onlyBounds", "just output minimum and maximum of the wanted observable and exit", false);
        TCLAP::SwitchArg onlyCentersSwitch("", "onlyCenters", "just output the centers of the WL bins and exit", false);
        TCLAP::SwitchArg onlyLERWExampleSwitch("", "onlyLERWExample", "just output a picture of erased loops", false);
        TCLAP::SwitchArg onlyPivotExampleSwitch("", "onlyPivotExample", "just output a picture of a pivot step", false);
        TCLAP::SwitchArg benchmarkSwitch("b", "benchmark", "perform benchmark", false);
        TCLAP::SwitchArg quietSwitch("q", "quiet", "quiet mode, log only to file (if specified) and not to stdout", false);

        // Add to the parser
        cmd.add(numArg);
        cmd.add(numWalkerArg);
        cmd.add(iterationsArg);
        cmd.add(sweepArg);
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
        cmd.add(lnfArg);
        cmd.add(flatnessArg);
        cmd.add(wangLandauBordersMArg);
        cmd.add(wangLandauBinsArg);
        cmd.add(wangLandauOverlapArg);
        cmd.add(typeArg);
        cmd.add(muArg);
        cmd.add(sigmaArg);
        cmd.add(widthArg);
        cmd.add(tasArg);
        cmd.add(passageTimeStartArg);
        cmd.add(svgArg);
        cmd.add(povArg);
        cmd.add(gpArg);
        cmd.add(threejsArg);
        cmd.add(dataPathArg);
        cmd.add(confPathArg);
        cmd.add(tmpPathArg);

        cmd.add(aklHeuristicSwitch);
        cmd.add(simpleSamplingSwitch);

        cmd.add(onlyBoundsSwitch);
        cmd.add(onlyCentersSwitch);
        cmd.add(onlyLERWExampleSwitch);
        cmd.add(onlyPivotExampleSwitch);

        cmd.add(benchmarkSwitch);

        cmd.add(quietSwitch);
        cmd.add(logfileArg);
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

        onlyBounds = onlyBoundsSwitch.getValue();
        onlyCenters = onlyCentersSwitch.getValue();
        onlyLERWExample = onlyLERWExampleSwitch.getValue();
        onlyPivotExample = onlyPivotExampleSwitch.getValue();
        if(onlyBounds + onlyCenters + onlyLERWExample + onlyPivotExample > 1)
        {
            LOG(LOG_ERROR) << "--only* are mutually exclusive";
            std::string list;
            if(onlyBounds)
                list += "--onlyBounds ";
            if(onlyCenters)
                list += "--onlyCenters ";
            if(onlyLERWExample)
                list += "--onlyLERWExample ";
            if(onlyPivotExample)
                list += "--onlyPivotExample ";
            LOG(LOG_ERROR) << "you activated: " << list;
            exit(1);
        }

        if(onlyBounds)
        {
            LOG(LOG_INFO) << "onlyBounds Mode";
        }

        if(onlyCenters)
        {
            LOG(LOG_INFO) << "onlyCenters Mode";
        }

        if(onlyLERWExample)
        {
            if(svgArg.getValue().empty())
            {
                LOG(LOG_ERROR) << "-s path needed to save the visualization";
                exit(1);
            }
            LOG(LOG_INFO) << "onlyLERWExample Mode";
        }

        if(onlyPivotExample)
        {
            if(svgArg.getValue().empty())
            {
                LOG(LOG_ERROR) << "-s path needed to save the visualization";
            }
            LOG(LOG_INFO) << "onlyPivotExample Mode";
        }


        // Get the value parsed by each arg.
        Logger::quiet = quietSwitch.getValue();
        Logger::verbosity = verboseArg.getValue();
        Logger::logfilename = logfileArg.getValue();

        // do not log, if there is no way to output
        if(Logger::quiet && Logger::logfilename.empty())
            Logger::verbosity = 0;

        LOG(LOG_INFO) << text;
        LOG(LOG_INFO) << "Verbosity                  " << Logger::verbosity;

        LOG(LOG_INFO) << "Version: " << VERSION;
        LOG(LOG_INFO) << "Compiled: " << __DATE__ << " " << __TIME__;

        LOG(LOG_INFO) << "Logging to " << Logger::logfilename;

        type = (walk_type_t) typeArg.getValue();
        LOG(LOG_INFO) << "Type                       " << TYPE_LABEL[type];

        mu = muArg.getValue();
        if(mu != 0.0)
        {
            LOG(LOG_INFO) << "mu                         " << mu;
        }
        sigma = sigmaArg.getValue();
        if(sigma != 1.0)
        {
            LOG(LOG_INFO) << "sigma                      " << sigma;
        }

        width = widthArg.getValue();
        if(width != 10)
        {
            LOG(LOG_INFO) << "width                      " << width;
        }
        tas = tasArg.getValue();
        if(tas != 1000)
        {
            LOG(LOG_INFO) << "tas                        " << tas;
        }

        steps = numArg.getValue();
        LOG(LOG_INFO) << "Number of steps            " << steps;

        numWalker = numWalkerArg.getValue();
        LOG(LOG_INFO) << "Number of walker           " << numWalker;

        d = dimArg.getValue();
        LOG(LOG_INFO) << "Dimension                  " << d;
        if(D_MAX && d > D_MAX)
        {
            LOG(LOG_ERROR) << "Dimension is larger than D_MAX = " << D_MAX << "\nCompile again with a bigger D_MAX (or D_MAX=0 for arbitrary dimensions)";
            exit(1);
        }

        bool aklHeuristic = aklHeuristicSwitch.getValue();
        int tmp = chAlgArg.getValue();
        if(tmp > 0)
            tmp = (tmp-1)*2 + 1;
        if(aklHeuristic && tmp > 0)
            tmp++;
        chAlg = (hull_algorithm_t) tmp;
        LOG(LOG_INFO) << "Convex Hull Algorithm      " << CH_LABEL[chAlg];

        seedRealization = seedRArg.getValue();
        LOG(LOG_INFO) << "Seed for the realization   " << seedRealization;

        seedMC = seedMCArg.getValue();
        LOG(LOG_INFO) << "Seed for the MC simulation " << seedMC;

        wantedObservable = (wanted_observable_t) wantedobservableArg.getValue();
        LOG(LOG_INFO) << "Wanted Observable          " << WANTED_OBSERVABLE_LABEL[wantedObservable];

        passageTimeStarts = passageTimeStartArg.getValue();
        LOG(LOG_INFO) << "passageTimeStart           " << passageTimeStarts;
        if(!passageTimeStarts.empty())
        {
            passageTimeStart = passageTimeStarts[0];
            if(passageTimeStarts.size() > 1)
            {
                LOG(LOG_WARNING) << "more than one passageTimeStart is only meaningful for simple sampling";
            }
            if(wantedObservable != WO_PASSAGE)
            {
                LOG(LOG_WARNING) << "This parameter is only used for -w 3 / --wantedObservable 3";
            }
        }

        svg_path = svgArg.getValue();
        sampling_method = (sampling_method_t) samplingMethodArg.getValue();
        LOG(LOG_INFO) << "Sampling Method            " << SAMPLING_METHOD_LABEL[sampling_method];

        sweep = sweepArg.getValue();
        if(sampling_method == SM_METROPOLIS || sampling_method == SM_METROPOLIS_PARALLEL_TEMPERING)
        {
            if(sweep == -1)
                sweep = steps;
            LOG(LOG_INFO) << "Number of MC per sweep     " << sweep;
        }

        iterations = iterationsArg.getValue();
        LOG(LOG_INFO) << "Number of MC sweeps        " << iterations;
        if(sampling_method == SM_WANG_LANDAU || sampling_method == SM_FAST_WANG_LANDAU)
            if(iterations > 10)
            {
                LOG(LOG_WARNING) << "One Wang Landau simulation will take some time, are you sure you want to repeat it " << iterations << " times?";
            }

        simpleSampling = simpleSamplingSwitch.getValue();
        if(simpleSampling && sampling_method == SM_METROPOLIS)
        {
            LOG(LOG_INFO) << "simple sampling            ";
        }
        else if(simpleSampling)
        {
            LOG(LOG_WARNING) << "The --simplesampling switch is a badly named. It just ensures that Metropolis is simulated at infinite temperature. It is useless for every other sampling method";
        }

        t_eq = t_eqArg.getValue();
        if(sampling_method == SM_METROPOLIS || sampling_method == SM_METROPOLIS_PARALLEL_TEMPERING)
            if(t_eq >= 0)
            {
                LOG(LOG_INFO) << "Set equilibration time to  " << t_eq;
            }

        t_eqMax = t_eqMaxArg.getValue();
        if(sampling_method == SM_METROPOLIS || sampling_method == SM_METROPOLIS_PARALLEL_TEMPERING)
        {
            LOG(LOG_INFO) << "Abort simulation if t_eq > " << t_eqMax;
        }

        if(!thetaArg.getValue().empty())
            theta = thetaArg.getValue()[0];
        else
            theta = 0.0;
        parallelTemperatures = thetaArg.getValue();
        if(!simpleSampling && sampling_method == SM_METROPOLIS)
        {
            if(parallelTemperatures.size() > 1)
            {
                LOG(LOG_ERROR) << "Do only specify one -T/--theta for this sampling method.";
                exit(1);
            }
            LOG(LOG_INFO) << "Theta                      " << theta;
        }
        if(sampling_method == SM_METROPOLIS_PARALLEL_TEMPERING)
        {
            if(parallelTemperatures.empty())
            {
                LOG(LOG_ERROR) << "No temperatures -T/--theta given, specify at least one";
                exit(1);
            }
            LOG(LOG_INFO) << "Thetas =                   {" << parallelTemperatures << "}";
        }

        lnf_min = lnfArg.getValue();
        flatness_criterion = flatnessArg.getValue();
        wangLandauBorders = wangLandauBordersMArg.getValue();
        wangLandauBins = wangLandauBinsArg.getValue();
        wangLandauOverlap = wangLandauOverlapArg.getValue();
        if(sampling_method == SM_WANG_LANDAU || sampling_method == SM_FAST_WANG_LANDAU)
        {
            if(wangLandauBorders.size() < 2)
            {
                LOG(LOG_ERROR) << "specify at least two borders: -e <lower> -e <upper>";
                exit(1);
            }
            std::sort(wangLandauBorders.begin(), wangLandauBorders.end());
            LOG(LOG_INFO) << "minimum ln(f):             " << lnf_min;
            if(sampling_method == SM_WANG_LANDAU)
            {
                LOG(LOG_INFO) << "flatness criterion:        " << flatness_criterion;
            }
            LOG(LOG_INFO) << "Borders of ranges for Wang Landau Sampling: \n#                 " << wangLandauBorders;
            LOG(LOG_INFO) << "Bins each range:           " << wangLandauBins;
            LOG(LOG_INFO) << "Overlap between ranges:    " << wangLandauOverlap;
        }

        parallel = parallelArg.getValue();
        if(sampling_method == SM_WANG_LANDAU || sampling_method == SM_FAST_WANG_LANDAU || sampling_method == SM_METROPOLIS_PARALLEL_TEMPERING)
        {
            LOG(LOG_INFO) << "CPUs to use                " << (parallel ? std::to_string(parallel) : "all");
        }
        if(parallel)
        {
            omp_set_num_threads(parallel);
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

        threejs_path = threejsArg.getValue();
        if(!threejs_path.empty())
        {
            LOG(LOG_INFO) << "Path to store the three.js " << threejs_path;
        }

        if(!dataPathArg.getValue().empty())
        {
            data_path_vector = dataPathArg.getValue();
            data_path = data_path_vector[0];
        }
        else
        {
            data_path = "out.dat";
        }
        if(sampling_method == SM_METROPOLIS_PARALLEL_TEMPERING)
        {
            data_path = "";
            if(parallelTemperatures.size() != data_path_vector.size())
            {
                LOG(LOG_ERROR) << "You need " << parallelTemperatures.size() << " paths, one for every -T / --theta, you have: " << data_path_vector.size();\
                exit(1);
            }
            else
            {
                LOG(LOG_INFO) << "Paths to store the data    {" << data_path_vector << "}";
            }
        }
        else
        {
            LOG(LOG_INFO) << "Path to store the data     " << data_path;
        }

        if(!confPathArg.getValue().empty())
        {
            conf_path_vector = confPathArg.getValue();
            conf_path = conf_path_vector[0];
        }
        else
        {
            conf_path = "";
        }
        if(sampling_method == SM_METROPOLIS_PARALLEL_TEMPERING)
        {
            if(conf_path_vector.size() && parallelTemperatures.size() != conf_path_vector.size())
            {
                LOG(LOG_ERROR) << "You need " << parallelTemperatures.size() << " paths, one for every -T / --theta, or none, you have: " << conf_path_vector.size();
                exit(1);
            }
            else if(!conf_path_vector.size())
            {
                conf_path_vector = std::vector<std::string>(parallelTemperatures.size(), "");
                LOG(LOG_INFO) << "Do not store the conf";
            }
            else
            {
                LOG(LOG_INFO) << "Paths to store the conf     {" << conf_path_vector << "}";
            }
        }
        else
        {
            LOG(LOG_INFO) << "Path to store the conf     " << conf_path;
        }

        tmp_path = tmpPathArg.getValue();
        LOG(LOG_INFO) << "Path for temporary files   " << tmp_path;
    }
    catch(TCLAP::ArgException &e)  // catch any exceptions
    {
        LOG(LOG_ERROR) << e.error() << " for arg " << e.argId();;
    }
}
