from numpy import arange

basename = "t{typ}_w{observable}_N{steps}_n{iterations}_x{seedMC}_y{seedR}_T{theta:.5f}"

sizes = (  32,  46,  64,  92, 128, 180, 256, 362, 512, 724, 1024, 1448, 2048)

parameters = {
    # what type:    1 random walk, 2 loop erased random walk
    "typ": 1,
    # random seed for Monte Carlo
    "seedMC": 1337,
    # random seed for initial configuration
    "seedR": 42,
    # how many iterations (i.e. sweeps) per theta and N
    "iterations": 5000,
    # dimension
    "dimension": 2,

    # list of cities
    "number_of_steps": sizes,

    # list of different thetas
    "thetas": [-100, -80, -60, -50, -40, -30, -20, -15, -10, -9, -7, -5, -4, -3, -2, -1],

    # where to save the temporary evaluation results
    "directory": "data",
    # where to save the data
    "rawData": "rawData",
    # where to save the raw data needed to reconstruct everything else
    "rawConf": "rawConf",

    # for which observable wen want the probabilities
    "observable": 1, # 1: surface area, 2: volume
    # algorithm for the convex hull
    "method": 2, # 1: qhull, 2: andrews, 4: jarvis
    # akl heuristic
    "akl": True
}
