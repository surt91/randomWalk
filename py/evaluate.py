import parameters as para

import evaluateMetropolis
import evaluateWangLandau

if __name__ == "__main__":
    if param.parameters["sampling"] == 1:
        evaluateMetropolis.run()
    elif param.parameters["sampling"] == 2:
        evaluateWangLandau.run()
