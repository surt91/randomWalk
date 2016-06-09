import sys
import logging

import parameters as param

import evaluateMetropolis
import evaluateWangLandau
import commonEvaluation

if __name__ == "__main__":
    if param.parameters["sampling"] == 1:
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
        evaluateMetropolis.run(ht)
    elif param.parameters["sampling"] == 2:
        evaluateWangLandau.run()

    commonEvaluation.cut_trans(param.parameters["S"])
    commonEvaluation.get_max_dist()
