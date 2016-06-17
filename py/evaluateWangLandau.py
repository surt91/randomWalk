#!/usr/bin/env python3

import logging
import gzip

import numpy as np
from scipy.interpolate import interp1d
from scipy.integrate import trapz

import parameters as param
from config import bootstrap, bootstrap_histogram, histogram_simple_error, SimulationInstance
from commonEvaluation import getMinMaxTime


logging.basicConfig(level=logging.INFO,
                format='%(asctime)s -- %(levelname)s :: %(message)s',
                datefmt='%d.%m.%YT%H:%M:%S')


def process_data(infiles, outformat):
    """Reads in the data, calculates the distribution from it, normalizes
    it, and writes it back to files.
    """
    centers = []
    data = []
    for infile in infiles:
        with gzip.open(infile+".gz", "rt") as f:
            even = True
            for l in f.readlines():
                if "#" in l:
                    continue
                if not l.split():
                        continue
                if even:
                    centers.append([float(i) for i in l.split()])
                    even = False
                else:
                    data.append([float(i) for i in l.split()])
                    even = True

    # sort them
    centers, data = (np.array(x) for x in zip(*sorted(zip(centers, data))))

    outfile = outformat.format("wl_raw")
    with open(outfile, "w") as f:
        f.write("# S err count(S) count(S)_err\n")
        for c, d in zip(centers, data):
            for l in zip(c, d):
                f.write("{} {}\n".format(*l))
            f.write("\n\n")

    # for identical centers, average their data and take the stderr
    processed_centers = set()
    preserve = np.ones(len(data), dtype=np.bool)
    stderr = np.zeros((len(data), len(data[0])))
    for i in range(len(centers)):
        corresponding_data = []
        if i in processed_centers:
            preserve[i] = False
            continue
        processed_centers.add(i)
        for n, c in enumerate(centers[i:], start=i):
            if c[0] == centers[i][0]:
                processed_centers.add(n)
                corresponding_data.append(data[n]-np.mean(data[n]))
        data[i] = np.mean(corresponding_data, axis=0)
        stderr[i] = np.std(corresponding_data, axis=0)

    centers = centers[preserve]
    data = data[preserve]
    stderr = stderr[preserve]

    stichInterpol(centers, data, stderr)

    outfile = outformat.format("wl_stiched")
    with open(outfile, "w") as f:
        f.write("# S err count(S) count(S)_err\n")
        for c, d, e in zip(centers, data, stderr):
            for l in zip(c, d, e):
                f.write("{} {} {}\n".format(*l))
            f.write("\n\n")

    # flatten and remove overlap
    #~ overlap = param.parameters["overlap"]
    #~ centers = [j for i in centers for j in i[overlap//2:-overlap//2]]
    #~ data = np.array([j for i in data for j in i[overlap//2:-overlap//2]])

    # flatten
    centers = centers.flatten()
    centers_err = [0 for i in range(len(centers))]
    data = data.flatten()
    stderr = stderr.flatten()

    # normalize
    data -= np.max(data)
    area = trapz(np.exp(data), centers)
    data -= np.log(area)

    outfile = outformat.format("WL")
    with open(outfile, "w") as f:
        f.write("# S ln(P(S)) ln(P(S)_err)\n")
        for d in zip(centers, centers_err, data, stderr):
            f.write("{} {} {} {}\n".format(*d))


def stichInterpol(centers, data, stderr):
    """Calculate the 'Z' values needed to stitch parts of the distribution
    together. This is calculated from the overlap of two neighboring
    ranges. Warns if there is not enough overlap.

    Uses spline interpolation to measure 'Z'.

    Applies the 'Z' values to the data to generate a continuous
    distribution from the single parts.
    """
    summed_err = 0
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
            logging.warning("not enough overlap ({}: {} -- {})".format(len(Z), max(centers[i]), min(centers[i+1])))

        z, err = bootstrap(Z)
        summed_err += err
        for j in range(len(data[i+1])):
            data[i+1][j] -= z
            stderr[i+1][j] += summed_err


def run():
    """Reads rawData files from a finished simulation, specified
    by 'parameters.py' in the same folder and evaluates it.
    """
    steps = param.parameters["number_of_steps"]
    d = param.parameters["rawData"]
    out = param.parameters["directory"]
    energies = param.parameters["energies"]

    outfiles = []

    for N in steps:
        outname = param.basename.format(steps=N, **param.parameters)
        outbase = "{}/{{}}_{}.dat".format(out, outname)
        if param.parameters["parallel"] is None:
            p = 1
        else:
            p = param.parameters["parallel"]
        num = len(energies[N])-1
        names = ["{}/{}.dat".format(d, SimulationInstance(steps=N, energy=list(energies[N][i:i+p+1]), first=not i, last=(i==num-1), **param.parameters).basename) for i in range(num)]

        logging.info("N = {}".format(N))
        getMinMaxTime(f for f in names)

        data = process_data(names,
                            outbase
                           )

    print("plot with gnuplot")
    print('p "{}" u 1:2:3 w ye'.format(outbase.format("WL")))


if __name__ == "__main__":
    logging.info("started Wang Landau evaluation")
    run()
