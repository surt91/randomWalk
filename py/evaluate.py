import sys
import logging

import parameters as param

import evaluateMetropolis
import evaluateWangLandau
import commonEvaluation

if __name__ == "__main__":
    sampling = param.parameters["sampling"]
    if sampling == 1:
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
            logging.info("Default: Using log-spaced histogram")
            ht = 2
        if not "--fast" in sys.argv:
            evaluateMetropolis.run(ht)
        else:
            logging.info("Fastmode: do not create new histograms from rawData")
    elif sampling == 2 or sampling == 3:
        evaluateWangLandau.run()

    commonEvaluation.cut_trans(param.parameters["S"])
    commonEvaluation.get_max_dist()
