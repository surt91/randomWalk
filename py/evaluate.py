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

    counts, bins = np.histogram(a.transpose()[2][3000:], 30, normed=True)
    centers = (bins[1:] + bins[:1])/2

    bs_mean, bs_err = bootstrap_histogram(a.transpose()[2][3000:], bins=bins, normed=True)
    #~ bs_mean = counts
    #~ bs_err = histogram_simple_error(counts)

    # only for uniform bins
    center_err = (bins[1] - bins[0]) / 4

    with open(outfile, "w") as f:
        f.write("# steps A A_err P(A) P(A)_err")
        for i, j, j_err in zip(centers, bs_mean, bs_err):
            #~ print(i, exp(i/theta) * j)
            try:
                ps = i/theta + log(j)
                # gaussian erroro propagation
                ps_err = 1/j * j_err + center_err/theta
            except ValueError:
                ps = 0
            else:
                f.write("{} {} {} {} {}\n".format(steps, i, center_err, ps, ps_err))


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
