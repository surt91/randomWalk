import gzip
import logging
from multiprocessing import Pool

import numpy as np
from scipy.interpolate import interp1d

import parameters as param


def getMinMaxTimeHelper(filename):
    time, mem, version = 0, 0, ""
    with gzip.open(filename+".gz", "rt") as f:
        for i in f.readlines():
            if "# Does not equilibrate" in i:
                break

            if "# Version" in i:
                s = i.split(":")[-1].strip()
                version = [s]
            if "# Compiled" in i:
                s = i.split(":")[1:]
                version.append(":".join(s).strip())
            if "# Started" in i:
                s = i.split(":")[1:]
                s = ":".join(s).strip()
                # ?
            if "# time in seconds" in i or "# time/sweep in seconds" in i:
                s = i.split(":")[-1].strip().strip("s")
                time = float(s)
            if "# max vmem: VmPeak" in i:
                s = i.split(":")[-1].strip().strip(" kB")
                mem = float(s)

    return time, mem, version

def getMinMaxTime(filenames):
    """Reads files given in first argument and collects metadata from
    them.

    Time needed per sweep
    vMemory needed
    git revison
    Date of compilation
    """
    with Pool() as p:
        times, mems, versions = zip(*p.map(getMinMaxTimeHelper, [f for f in filenames]))

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
        if param.parameters["sampling"] == 1 or param.parameters["sampling"] == 4:
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
        if param.parameters["sampling"] == 1 or param.parameters["sampling"] == 4:
            prefix = "whole"
        elif param.parameters["sampling"] == 2 or param.parameters["sampling"] == 3:
            prefix = "WL"
        else:
            raise

        whole_distribution_file = "{}/{}_{}.dat".format(param.parameters["directory"], prefix, name)
        a = np.loadtxt(whole_distribution_file)
        a = a.transpose()

        idx = np.argmax(a[2])
        maxX = a[0][idx]
        maxY = a[2][idx]
        xErr = a[1][idx]
        yErr = a[3][idx]

        with open("{}/{}.dat".format(param.parameters["directory"], "max"), "a") as f:
            f.write("{} {} {} {} {}\n".format(N, maxX, xErr, maxY, yErr))
