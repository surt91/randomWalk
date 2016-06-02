import numpy as np

basename = "m{sampling}_t{typ}_w{observable}_d{dimension}_N{steps}_n{iterations:.0f}"
noNname = "m{sampling}_t{typ}_w{observable}_d{dimension}_n{iterations:.0f}"
basetheta = basename + "_T{theta:.5f}"
basee = basename + "_e{estart:.0f}-{eend:.0f}"

sizes = (32, 64)

# thetas for the system sizes, missing sizes will get the 0 entry
thetas = { 32: (1, 10, float("inf"), -10, -5, -4, -3, -2, -1),
           64: (1, 3,  10, float("inf"), -20, -13, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1),
         }

t_eq = {
       }

t_corr = {
         }

energies = {    32: np.linspace(5, 600, 5),
                64: np.linspace(20, 6000, 12),
               128: np.linspace(20, 12000, 20),
               256: np.linspace(20, 25000, 20),
               512: np.linspace(20, 50000, 30),
              1024: np.linspace(20, 100000, 40),
              2048: np.linspace(20, 200000, 40),
           }

# subdivide the beginning, to ease the peak
energies[1024] = np.insert(energies[1024], 1, np.linspace(energies[1024][0]+200, energies[1024][1], 10, endpoint=False))
energies[2048] = np.insert(energies[2048], 1, np.linspace(energies[2048][0]+200, energies[2048][1], 10, endpoint=False))

parameters = {
    # what type
    # 1 random walk
    # 2 loop erased random walk
    # 3 self avoiding random walk
    # 4 random direction
    # 5 Gaussian walk
    # 6 Levy flight
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

    # dict[N] of different thetas
    "thetas": thetas,

    # dict[N][theta] of equilibration times
    "t_eq": t_eq,

    # dict[N][theta] of estimated autocorrelation times
    # will be used to take by factor t_corr more samples
    "t_corr": t_corr,

    # dict[N] list of energy borders, used for Wang landau Sampling
    "energies": energies,

    "nbins": 100,
    "overlap": 20,
    "overlap_direction": "right",
    "lnf": 1e-8,
    "flatness": 0.8,

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

    # how many cpus (only for wang landau), None means all
    # for HERO: 1 - 12
    "parallel": None,

    "basename": basename,
    "noNname": noNname,
    "basetheta": basetheta,
    "basee": basee,

    # parameter for evaluation of the rate function
    "S": [0.02, 0.04, 0.06, 0.08, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
}
