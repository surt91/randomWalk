import sys
import os
import logging
import multiprocessing

import parameters as param

import evaluateMetropolis
import evaluateWangLandau
import evaluateSimple
import commonEvaluation

if __name__ == "__main__":
    os.makedirs("data", exist_ok=True)
    try:
        parallelness = multiprocessing.cpu_count()
    except NotImplementedError:
        parallelness = 1

    for n, i in enumerate(sys.argv):
        if i == "-p":
            if len(i) > 2:
                parallelness = int(i[2:])
            else:
                parallelness = int(sys.argv[n+1])

    logging.info("Using {}-way parallelism".format(parallelness))

    if not "--fast" in sys.argv:
        sampling = param.parameters["sampling"]
        if sampling == 1 or sampling == 4:
            if "--lin" in sys.argv:
                ht = 1
                logging.info("Using equi-spaced histogram")
            elif "--log" in sys.argv:
                ht = 2
                logging.info("Using logarithmic histogram")
            elif "--flat" in sys.argv:
                ht = 3
                logging.info("Using percentile-based histogram")
            else:
                logging.info("Default: Using lin-spaced histogram")
                ht = 1

            evaluateMetropolis.run(ht, parallelness=parallelness)

        elif sampling == 2 or sampling == 3:
            evaluateWangLandau.run(parallelness=parallelness)

        elif sampling == 0:
            evaluateSimple.run(parallelness=parallelness)
        else:
            logging.error("unknown sampling method")

    else:
        logging.info("Fastmode: do not create new histograms from rawData")

    if sampling != 0:
        commonEvaluation.cut_trans(param.parameters["S"])
    commonEvaluation.get_max_dist()

    logging.info("finished")
