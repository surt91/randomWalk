import parameters as param

import evaluateMetropolis
import evaluateWangLandau
import commonEvaluation

if __name__ == "__main__":
    if param.parameters["sampling"] == 1:
        evaluateMetropolis.run()
    elif param.parameters["sampling"] == 2:
        evaluateWangLandau.run()

    commonEvaluation.cut_trans(param.parameters["S"])
