#!/usr/bin/env python3

import logging
import gzip

import numpy as np
from scipy.interpolate import interp1d
from scipy.integrate import trapz

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

    # sort them
    centers, data = (list(x) for x in zip(*sorted(zip(centers, data))))

    stichInterpol(centers, data)

    # flatten and remove overlap
    #~ overlap = param.parameters["overlap"]
    #~ centers = [j for i in centers for j in i[overlap//2:-overlap//2]]
    #~ data = np.array([j for i in data for j in i[overlap//2:-overlap//2]])

    # flatten
    centers = [j for i in centers for j in i]
    data = np.array([j for i in data for j in i])

    # normalize
    data -= np.max(data)
    area = trapz(np.exp(data), centers)
    data -= np.log(area)

    with open(outfile, "w") as f:
        f.write("# S err P(S) P(S)_err\n")
        for d in zip(centers, data):
            f.write("{} {}\n".format(*d))


def stichInterpol(centers, data):
    for i in range(len(data)-1):

        try:
            spline_1 = interp1d(centers[i], data[i], kind='cubic')
        except ValueError:
            logging.error("too few samples")
            continue
        except TypeError:
            logging.error("too few samples")
            continue

        # calculate for every x the difference to y(x) - spline_1(x)
        Z = [y - spline_1(x) for x, y in zip(centers[i+1], data[i+1]) if min(centers[i]) < x < max(centers[i])]

        # not enough overlap
        if len(Z) < 3:
            logging.warning("not enough overlap")

        z, err = bootstrap(Z)
        for j in range(len(data[i+1])):
            data[i+1][j] -= z


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
