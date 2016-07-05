import numpy as np

basename = "m{sampling}_t{typ}_w{observable}_d{dimension}_N{steps}_n{iterations:.0f}"
noNname = "m{sampling}_t{typ}_w{observable}_d{dimension}_n{iterations:.0f}"
basetheta = basename + "_T{theta:.5f}"
basee = basename + "_e{estart:.0f}-{eend:.0f}"

sizes = (32, 64, 128, 256, 512, 1024, 2048)
sweep = {n: n**0.5 for n in sizes}

# thetas for the system sizes, missing sizes will get the 0 entry
thetas = { 32: (1, 10, float("inf"), -10, -5, -4, -3, -2, -1),
           64: (1, 3,  10, float("inf"), -20, -13, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1),
         }

t_eq = {
       }

t_corr = {
         }

# target for rate function: Phi_target -> max_energy = Phi_target*N**d
energies = {     32: np.linspace(   10,     10000,  6),
                 64: np.linspace(   20,     80000, 12),
                128: np.linspace(  800,   1000000, 24),
                256: np.linspace( 5000,  12000000, 48),
                384: np.linspace( 9000,  62000000, 48),
                512: list(np.linspace(   12000,    100000, 10, endpoint=False))
                   + list(np.linspace(  100000,    160000, 10, endpoint=False))
                   + list(np.linspace(  160000,   1600000, 10, endpoint=False))
                   + list(np.linspace( 1600000,  16000000, 10, endpoint=False))
                   + list(np.linspace(16000000, 132000000, 40)),
               1024: list(np.linspace(   24000,    400000, 10, endpoint=False))
                   + list(np.linspace(  400000,    600000, 10, endpoint=False))
                   + list(np.linspace(  600000,   6000000, 10, endpoint=False))
                   + list(np.linspace( 6000000,  60000000, 10, endpoint=False))
                   + list(np.linspace(60000000, 932000000, 40)),
               2048: list(np.linspace(    48000,    1800000, 10, endpoint=False))
                   + list(np.linspace(  1800000,    2800000, 10, endpoint=False))
                   + list(np.linspace(  2800000,   28000000, 10, endpoint=False))
                   + list(np.linspace( 28000000,  280000000, 10, endpoint=False))
                   + list(np.linspace(280000000, 5932000000, 40)),
           }

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

    # number of walkers
    "number_of_walkers": 1;
    # list of lengths of the walks
    "number_of_steps": sizes,
    # number of MC tries per sweep
    "sweep": sweep,

    # dict[N] of different thetas
    "thetas": thetas,

    # dict[N][theta] of equilibration times
    "t_eq": t_eq,
    "t_eq_max": 1e4,

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
    # 1: qhull,
    # 2: andrews (only 2D)
    "method": 2,

    # akl heuristic
    # only available in d=2, yet
    "akl": False,
    # which smapling methd
    # 1: Metropolis
    # 2: Wang Landau
    # 3: Fast 1/t Wang Landau
    # 4: Metropolis with parallel tempering
    "sampling": 2,

    # how many cpus (only for wang landau), None means all
    # for HERO: 1 - 12
    "parallel": None,

    "basename": basename,
    "noNname": noNname,
    "basetheta": basetheta,
    "basee": basee,

    # parameter for evaluation of the rate function
    "S":     [0.02, 0.04, 0.06, 0.08, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0],
    # N to begin the fit (one entry per value of S)
    "N_min": [   0,    0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0],
}
