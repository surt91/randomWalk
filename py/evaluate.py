#!/usr/bin/env python3

from math import exp, log

import numpy as np


def getDistribution(infile, outfile, theta):
    a = np.loadtxt(infile)
    counts, bins = np.histogram(a.transpose()[2][30000:], 30)
    centers = (bins[1:] + bins[:1])/2

    with open(outfile, "w") as f:
        for i, j in zip(centers, counts):
            #~ print(i, exp(i/theta) * j)
            try:
                ps = i/theta + log(j)
            except ValueError:
                ps = 0
            else:
                f.write("{} {}\n".format(i, ps))


if __name__ == "__main__":
    thetas = [-100, -80, -50, -40, -30, -25, -20, -17, -15, -13, -10, -9, -7, -5, -4, -3, -2]

    for T in thetas:
        getDistribution("data_T{}.dat".format(T), "out_T{}.dat".format(T), T)

    print("plot with gnuplot")
    print("p " + ", ".join('"out_T{}.dat"'.format(T) for T in thetas))
