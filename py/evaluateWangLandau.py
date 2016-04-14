#!/usr/bin/env python3

import logging

import numpy as np

import parameters as param
from config import bootstrap, bootstrap_histogram, histogram_simple_error


logging.basicConfig(level=logging.INFO,
                format='%(asctime)s -- %(levelname)s :: %(message)s',
                datefmt='%d.%m.%YT%H:%M:%S')
logging.info("started")


def process_data(infile, outfile):
    try:
        a = np.loadtxt(infile+".gz")
    except FileNotFoundError:
        try:
            a = np.loadtxt(infile)
        except FileNotFoundError:
            logging.error("cannot find " + infile)
            return

    centers = a[0]
    centers_err = [(centers[1] - centers[0]) / 2 for _ in centers]
    data = a[1:]

    # normalize before bootstrapping
    for n in range(len(data)):
        data[n] -= np.max(data[n])
        area = np.trapz(np.exp(data[n]), centers)
        data[n] -= np.log(area)

    data = data.transpose()

    bs_mean = []
    bs_err = []
    for n, c in enumerate(centers):
        d, e = bootstrap(data[n])
        bs_mean.append(d)
        bs_err.append(e)

    with open(outfile, "w") as f:
        f.write("# S P(S) P(S)_err\n")
        for d in zip(centers, centers_err, bs_mean, bs_err):
            f.write("{} {} {} {}\n".format(*d))


def run():
    steps = param.parameters["number_of_steps"]
    d = param.parameters["rawData"]
    out = param.parameters["directory"]

    outfiles = []

    for N in steps:
        name = param.basename.format(steps=N,
                                       theta=0,
                                       **param.parameters
                                       )
        outname = "{}/WL_{}.dat".format(out, name)
        data = process_data("{}/{}.dat".format(d, name),
                            outname
                            )

    print("plot with gnuplot")
    print('p "{}" u 1:3:2:4 w xye'.format(outname))


if __name__ == "__main__":
    run()
