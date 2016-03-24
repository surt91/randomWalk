#!/usr/bin/env python3

from math import exp, log

import numpy as np

import parameters as param

def getDistribution(infile, outfile, theta, steps):
    a = np.loadtxt(infile)
    counts, bins = np.histogram(a.transpose()[2][30000:], 30, normed=True)
    centers = (bins[1:] + bins[:1])/2

    with open(outfile, "w") as f:
        f.write("# steps A P(A)")
        for i, j in zip(centers, counts):
            #~ print(i, exp(i/theta) * j)
            try:
                ps = i/theta + log(j)
            except ValueError:
                ps = 0
            else:
                f.write("{} {} {}\n".format(steps, i, ps))


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
    print("p " + ", ".join(i + " u 2:3" for i in outfiles))


if __name__ == "__main__":
    run()
