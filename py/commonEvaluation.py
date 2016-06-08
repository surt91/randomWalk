import gzip
import logging

import numpy as np
from scipy.interpolate import interp1d

import parameters as param


def getMinMaxTime(filenames):
    times = []
    mems = []
    versions = []
    for filename in filenames:
        with gzip.open(filename+".gz", "rt") as f:
            for i in f.readlines():
                if "# Does not equilibrate" in i:
                    break

                if "# Version" in i:
                    s = i.split(":")[-1].strip()
                    versions.append([s])
                if "# Compiled" in i:
                    s = i.split(":")[1:]
                    versions[-1].append(":".join(s).strip())
                if "# Started" in i:
                    s = i.split(":")[1:]
                    s = ":".join(s).strip()
                    # ?
                if "# time in seconds" in i or "# time/sweep in seconds" in i:
                    s = i.split(":")[-1].strip().strip("s")
                    times.append(float(s))
                if "# max vmem: VmPeak" in i:
                    s = i.split(":")[-1].strip().strip(" kB")
                    mems.append(float(s))
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


def cut_trans(s, pre="tran"):
    """Transpose. Get in-between values by interpolation."""
    # get values near s from all distributions (all systemsizes)
    P = {}
    P_err = {}
    for N in param.parameters["number_of_steps"]:
        outname = param.basename.format(steps=N, **param.parameters)
        if param.parameters["sampling"] == 1:
            prefix = "whole"
        elif param.parameters["sampling"] == 2:
            prefix = "WL"
        else:
            raise

        # FIXME: should be actual maximum, this is only correct up to a constant
        d = param.parameters["dimension"]
        if param.parameters["observable"] == 1:
            d -= 1
        S_max = N ** d

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
