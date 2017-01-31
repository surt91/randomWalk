import numpy as np

basename = "m{sampling}_t{typ}_w{observable}_d{dimension}_N{steps}_n{iterations:.0f}"
noNname = "m{sampling}_t{typ}_w{observable}_d{dimension}_n{iterations:.0f}"
basetheta = basename + "_T{theta:.5f}"
basee = basename + "_e{estart:.0f}-{eend:.0f}"

sizes = (32, 64, 128, 256, 512, 1024, 2048)

# thetas for the system sizes, missing sizes will get the 0 entry
thetas = {
         }

t_eq = {
       }

t_corr = {
         }

energies = {    32: [1, 20, 40, 80, 200, 400, 1000, 1500],
                64: [1, 20, 40, 80, 200, 400, 1000, 2500, 6000],
               128: [2, 20, 40, 80, 200, 400, 1000, 2500, 6000, 12000, 25000],
               256: [8, 20, 40, 80, 200, 400, 1000, 2500, 6000, 12000, 25000, 60000],
               512: [15, 40, 80, 200, 400, 1000, 2500, 6000, 12000, 25000, 60000, 120000],
              1024: [25, 40, 80, 200, 400, 1000, 2500, 6000, 12000, 25000, 60000, 120000, 240000],
              2048: [50, 80, 200, 400, 1000, 2500, 6000, 12000, 25000, 60000, 120000, 240000, 480000],
              2048: [50, 80, 200, 400, 1000, 2500, 6000, 12000, 25000, 60000, 120000, 240000, 480000],
           }

#S = [0.02, 0.04, 0.06, 0.08, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]

parameters = {
    # what type
    # 1 random walk
    # 2 loop erased random walk
    # 3 self avoiding random walk
    # 4 random direction
    # 5 Gaussian walk
    # 6 Levy flight
    "typ": 5,
    # random seed for Monte Carlo
    "seedMC": 1337,
    # random seed for initial configuration
    "seedR": 42,
    # how many iterations (i.e. sweeps) per theta and N
    "iterations": 4,
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
    "lnf": 1e-5,
    "flatness": 0.8,

    # where to save the temporary evaluation results
    "directory": "data",
    # where to save the data
    "rawData": "rawData",
    # where to save the raw data needed to reconstruct everything else
    "rawConf": False,

    # for which observable wen want the probabilities
    # 1: surface area
    # 2: volume
    # 3: passage times
    "observable": 2,

    # algorithm for the convex hull
    # qhull seems to be fastest, at least for big number of steps
    # for small number of steps, andrews is faster
    "method": 2,  # 1: qhull, 2: andrews, 4: jarvis

    # akl heuristic
    # only available in d=2, yet
    "akl": True,
    # which smapling methd
    # 1: Metropolis
    # 2: Wang Landau
    # 3: fast Wang Landau and Entropic sampling
    # 4: parallel tempering
    "sampling": 3,

    # how many cpus (only for wang landau), None means all
    # for HERO: 1 - 12
    "parallel": None,

    "basename": basename,
    "noNname": noNname,
    "basetheta": basetheta,
    "basee": basee,

    # parameter for evaluation of the rate function
    "S":     [0.02, 0.03, 0.035, 0.045, 0.05, 0.06, 0.08, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0],
    "N_min": [ 256,  128,   128,   128,    0,    0,    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0],

    # range for the gaussian fit to the peak on a rescaled axis
    "fit_xrange": {
                        32: (0.5, 1.5),
                        64: (0.5, 1.5),
                       128: (0.7, 1.5),
                       256: (0.7, 1.7),
                       512: (0.7, 1.7),
                      1024: (0.8, 1.6),
                      2048: (0.9, 1.5),
                  },
}
