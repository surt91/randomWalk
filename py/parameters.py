from numpy import arange

basename = "m{sampling}_t{typ}_w{observable}_d{dimension}_N{steps}_n{iterations}_x{seedMC}_y{seedR}_T{theta:.5f}"

sizes = (32, 64, 128, 256)

# thetas for the system sizes, missing sizes will get the 0 entry
thetas = {    0: (-1000, -100, -30, -20, -17, -13, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, -0.7, -0.5, 5, 10, 100),
            128: (-1000, -500, -200, -100, -50, -30, -25, -20, -17, -15, -13, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, -0.7, -0.5, 5, 10, 100),
            256: (-1000, -500, -200, -100, -50, -40, -30, -27, -25, -22, -20, -17, -15, -13, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, -0.7, -0.5, 5, 10, 100),
         }

parameters = {
    # what type
    # 1 random walk
    # 2 loop erased random walk
    # 3 self avoiding random walk
    # 4 random direction
    # 5 Gaussian walk
    "typ": 2,
    # random seed for Monte Carlo
    "seedMC": 1337,
    # random seed for initial configuration
    "seedR": 42,
    # how many iterations (i.e. sweeps) per theta and N
    "iterations": 10**4,
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
    # qhull seems to be fastest, at least for big number of steps
    # for small number of steps, andrews is faster
    "method": 2,  # 1: qhull, 2: andrews, 4: jarvis

    # akl heuristic
    # only available in d=2, yet
    "akl": False,
    # which smapling methd
    "sampling": 1,  # 1: metropolis, 2: wang landau

    # how many cpus (only for wang landau)
    # for HERO: 1 - 12
    "parallel": None,
}
