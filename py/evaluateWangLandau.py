#!/usr/bin/env python3

import logging
import gzip

import numpy as np

import parameters as param
from config import bootstrap, bootstrap_histogram, histogram_simple_error


logging.basicConfig(level=logging.INFO,
                format='%(asctime)s -- %(levelname)s :: %(message)s',
                datefmt='%d.%m.%YT%H:%M:%S')
logging.info("started")


def process_data(infile, outfile):
    with gzip.open(infile+".gz", "rt") as f:
        even = True
        centers = []
        data = []
        for l in f.readlines():
            if "#" in l:
                continue
            if even:
                centers.append([float(i) for i in l.split()])
                even = False
            else:
                data.append([float(i) for i in l.split()])
                even = True

    # TODO: stich it together -> overlap needed
    # TODO: normalize
    centers = [j for i in centers for j in i]
    data = [j for i in data for j in i]

    with open(outfile, "w") as f:
        f.write("# S err P(S) P(S)_err\n")
        for d in zip(centers, data):
            f.write("{} {}\n".format(*d))


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
