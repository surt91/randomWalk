import gzip
import logging
from multiprocessing import Pool

import numpy as np
from scipy.interpolate import interp1d
from scipy.integrate import trapz

import parameters as param


def getMinMaxTimeHelper(filename):
    theta = float("inf")
    time, mem, version = 0, 0, ""
    pt_acc = float("nan")
    reject = float("nan")
    tries = float("nan")
    with gzip.open(filename+".gz", "rt") as f:
        for i in f:
            # first ask, if this is a comment -> abort fast
            if i and i[0] != "#":
                continue

            elif "# Does not equilibrate" in i:
                break

            elif "theta=" in i:
                theta = float(i.split("=")[1].split(" ")[0].strip())
            elif "# Version" in i:
                s = i.split(":")[-1].strip()
                version = [s]
            elif "# Compiled" in i:
                s = i.split(":")[1:]
                version.append(":".join(s).strip())
            elif "# Started" in i:
                s = i.split(":")[1:]
                s = ":".join(s).strip()
                # ?
            elif "# time in seconds" in i or "# time/sweep in seconds" in i:
                s = i.split(":")[-1].strip().strip("s")
                time = float(s)
            elif "# max vmem: VmPeak" in i:
                s = i.split(":")[-1].strip().strip(" kB")
                mem = float(s)

            elif "# proposed changes" in i:
                s = i.split(":")[-1].strip()
                accept = float(s)
            elif "# rejected changes" in i:
                s = i.split(":")[-1].strip().split(" ")[0].strip()
                reject = float(s)

    return theta, time, mem, version, tries, reject

def getMinMaxTime(filenames, parallelness=1):
    """Reads files given in first argument and collects metadata from
    them.

    Time needed per sweep
    vMemory needed
    git revison
    Date of compilation
    """
    with Pool(parallelness) as p:
        theta, times, mems, versions, tries, reject = zip(*p.map(getMinMaxTimeHelper, [f for f in filenames]))

    try:
        logging.info("time/sweep between {:.2f}s - {:.2f}s".format(min(times), max(times)+0.0051))
    except ValueError:
        logging.info("No time measured")
    try:
        logging.info("memory between {:.0f}kB - {:.0f}kB".format(min(mems), max(mems)))
    except ValueError:
        logging.info("No memory measured")
    try:
        logging.info("used versions: {}".format(", ".join(set(str(i) for i in versions))))
    except ValueError:
        logging.info("No version information")

    return theta, times, mems, versions, tries, reject


def factorial(d):
    if d == 0:
        return 1
    return d * factorial(d-1)


def getMaximumS(N, d, observable, typ):
    if observable == 1:
        d -= 1
    # if this is a lattice walk, calculate the actual maximum
    if typ in (1, 2, 3):
        # special case circumference in d=2
        if d == 1:
            S_max = 2*N
        else:
            S_max = (N/d)**d / factorial(d)
    # otherwise give something propotional
    else:
        S_max = N**d
    return S_max


def getMaximumSForGnuplot(d, observable, typ):
    if observable == 1:
        d -= 1
    # if this is a lattice walk, calculate the actual maximum
    if typ in (1, 2, 3):
        # special case circumference in d=2
        if d == 1:
            S_max = "2*x"
        else:
            S_max = "(x/d)**d / {}.".format(factorial(d))
    # otherwise give something propotional
    else:
        S_max = "x**d"
    return S_max


def cut_trans(s, pre="tran"):
    """Transpose. Get in-between values by interpolation."""
    # get values near s from all distributions (all systemsizes)
    P = {}
    P_err = {}
    for N in param.parameters["number_of_steps"]:
        outname = param.basename.format(steps=N, **param.parameters)
        if param.parameters["sampling"] == 1 or param.parameters["sampling"] == 4 or param.parameters["sampling"] == 5:
            prefix = "whole"
        elif param.parameters["sampling"] == 2 or param.parameters["sampling"] == 3:
            prefix = "WL"
        else:
            raise

        d = param.parameters["dimension"]
        S_max = getMaximumS(N, d, param.parameters["observable"], param.parameters["typ"])

        fname = "{}/{}_{}.dat".format(param.parameters["directory"], prefix, outname)
        P[N] = {}
        P_err[N] = {}

        S = np.loadtxt(fname).transpose()
        S[0] /= S_max
        for x in s:
            # only take the 10 values next to s
            idx = np.argmin(np.abs(S[0]-x))
            tmp = S[:,max(0, idx-5):idx+5]
            # spline interpolate p(s) between the measured s_i
            spline = interp1d(tmp[0], tmp[2], kind='cubic')
            try:
                P[N][x] = - spline(x) / N
                # take max error of s_i
                P_err[N][x] = np.max(tmp[3]) / N
            except ValueError:
                P[N][x] = float("NaN")
                P_err[N][x] = float("NaN")


    # output a T p(s) err file
    fname = param.noNname + "_S{s}"
    for x in s:
        name = fname.format(s=x, **param.parameters)
        name = "{}/{}_{}.dat".format(param.parameters["directory"], pre, name)
        with open(name, "w") as f:
            f.write("# N p(s) err\n")
            for N in param.parameters["number_of_steps"]:
                f.write("{} {} {}\n".format(N, P[N][x], P_err[N][x]))


def get_max_dist():
    """Get Maxima of the distributions.
    Used for experimental extraction of scaling exponents and corrections
    to scaling.
    """
    #~ maxX, maxY, xErr, yErr
    with open("{}/{}.dat".format(param.parameters["directory"], "max"), "w") as f:
        f.write("# N maxX err maxY err\n")
    for N in param.parameters["number_of_steps"]:
        name = param.basename.format(steps=N, **param.parameters)
        if param.parameters["sampling"] == 1 or param.parameters["sampling"] == 4 or param.parameters["sampling"] == 5:
            prefix = "whole"
        elif param.parameters["sampling"] == 2 or param.parameters["sampling"] == 3:
            prefix = "WL"
        else:
            raise

        whole_distribution_file = "{}/{}_{}.dat".format(param.parameters["directory"], prefix, name)
        a = np.loadtxt(whole_distribution_file)
        a = a.transpose()

        idx = np.argmax(a[1])
        maxX = a[0][idx]
        maxY = a[1][idx]
        xErr = 0
        yErr = a[2][idx]

        with open("{}/{}.dat".format(param.parameters["directory"], "max"), "a") as f:
            f.write("{} {} {} {} {}\n".format(N, maxX, xErr, maxY, yErr))


def getMeanFromDist(centers, data):
    """Takes a log-distribution as input and calculates its mean and variance.

    Errors are widly overestimated and useless.
    TODO: bootstrap error estimates
    """
    mean = trapz(np.multiply(centers, np.exp(data)), centers)
    return mean

def getVarFromDist(centers, data):
    mean = getMeanFromDist(centers, data)
    var = trapz(np.multiply(np.power(centers, 2), np.exp(data)), centers) - mean**2
    return var


def reduce_distribution(num):
    """Take the datapoints from the input array and return 'num' (roughly) equally spaced datapoints.

    This is meant as preprocessing for plotting.
    """
    raise NotImplementedError
