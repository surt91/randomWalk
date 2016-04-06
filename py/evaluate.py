#!/usr/bin/env python3

from math import exp, log

import numpy as np

import parameters as param
from config import bootstrap_histogram, histogram_simple_error


def getDistribution(infile, outfile, theta, steps):
    try:
        a = np.loadtxt(infile+".gz")
    except FileNotFoundError:
        try:
            a = np.loadtxt(infile)
        except FileNotFoundError:
            print("cannot find " + self.filename)
            return

    col = param.parameters["observable"]
    counts, bins = np.histogram(a.transpose()[col], 50, density=True)
    centers = (bins[1:] + bins[:-1])/2
    centers_err = [(centers[i] - bins[i]) / 2 for i in range(len(centers))]

    # errors are not good, we need to include the autocorrelation time
    bs_mean, bs_err = bootstrap_histogram(a.transpose()[col], bins=bins, density=True)
    #bs_err = histogram_simple_error(counts)

    print(outfile)

    with open(outfile, "w") as f:
        f.write("# steps S S_err P(S) P(S)_err\n")
        for s, s_err, pts, pts_err in zip(centers, centers_err, bs_mean, bs_err):
            #~ print(i, exp(i/theta) * j)
            try:
                ps = s/theta + log(pts)
                # gaussian erroro propagation
                ps_err = 1/pts * pts_err + s_err/theta
                if ps_err > s_err:
                    # ignore too noisy bins
                    raise ValueError
            except ValueError:
                ps = 0
            else:
                f.write("{} {} {} {} {}\n".format(steps, s, s_err, ps, ps_err))
                #f.write("{} {} {} {} {}\n".format(steps, s, s_err, pts, pts_err))


def run():
    thetas = param.parameters["thetas"]
    steps = param.parameters["number_of_steps"]
    d = param.parameters["rawData"]
    out = param.parameters["directory"]

    outfiles = []

    for N in steps:
        for T in thetas:
            name = param.basename.format(steps=N,
                                         theta=T,
                                         typ=param.parameters["typ"],
                                         observable=param.parameters["observable"],
                                         iterations=param.parameters["iterations"],
                                         seedMC=param.parameters["seedMC"],
                                         seedR=param.parameters["seedR"]
                                         )
            getDistribution("{}/{}.dat".format(d, name), "{}/dist_{}.dat".format(out, name), T, N)
            outfiles.append('"{}/dist_{}.dat"'.format(out, name))


    print("plot with gnuplot")
    print("p " + ", ".join(i + " u 2:4:3:5 w xyerr" for i in outfiles))


if __name__ == "__main__":
    run()
