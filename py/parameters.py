from numpy import arange

basename = "m{sampling}_t{typ}_w{observable}_N{steps}_n{iterations}_x{seedMC}_y{seedR}_T{theta:.5f}"

#~ sizes = (  32,  46,  64,  92, 128, 180, 256, 362, 512, 724, 1024, 1448, 2048)
sizes = (200,)
thetas = (-100, -80, -50, -40, -35, -30, -27, -24, -22, -20, -19, -18, -17, -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -3.5, -2)

parameters = {
    # what type
    "typ": 1,  #1 random walk, 2 loop erased random walk, 3 self avoiding random walk
    # random seed for Monte Carlo
    "seedMC": 1337,
    # random seed for initial configuration
    "seedR": 42,
    # how many iterations (i.e. sweeps) per theta and N
    "iterations": 100000,
    # dimension
    "dimension": 2,

    # list of lengths of the walks
    "number_of_steps": sizes,

    # list of different thetas
    "thetas": thetas,

    # where to save the temporary evaluation results
    "directory": "data",
    # where to save the data
    "rawData": "rawData",
    # where to save the raw data needed to reconstruct everything else
    "rawConf": False,

    # for which observable wen want the probabilities
    "observable": 2,  # 1: surface area, 2: volume
    # algorithm for the convex hull
    "method": 2,  # 1: qhull, 2: andrews, 4: jarvis
    # akl heuristic
    "akl": False,
    # which smapling methd
    "sampling": 1,  # 1: metropolis, 2: wang landau
}
