#!/usr/bin/env python3

import logging
import gzip

import numpy as np
from scipy.interpolate import interp1d
from scipy.integrate import trapz

import parameters as param
from config import bootstrap, bootstrap_histogram, histogram_simple_error, SimulationInstance
from commonEvaluation import getMinMaxTime, getMeanFromDist, getVarFromDist


logging.basicConfig(level=logging.INFO,
                format='%(asctime)s -- %(levelname)s :: %(message)s',
                datefmt='%d.%m.%YT%H:%M:%S')


def readData(filenames, outformat=None):
    """Reads the given files and returns a list with n entries each full
    of fragments sufficient for one distribution.

    The raw data format consists of one file for each energy range,
    each file having "iterations" independent results.
    This function reads the files and "transposes" the format, i.e.,
    the output is are 'center' and 'data' arrays, with "iterations"
    many entries, containing an array of ranges containing the actual data:

    data[iteration][range(0..number_of_out_files)][bins(0..nbins+overlap)]
    """
    iterations = param.parameters["iterations"]
    centers = [[] for _ in range(iterations)]
    data = [[] for _ in range(iterations)]
    idx = None
    for infile in filenames:
        with gzip.open(infile+".gz", "rt") as f:
            even = True
            comments = 0
            for n, l in enumerate(f.readlines()):
                if "#" in l or "Compiled:" in l:
                    comments += 1
                    continue
                if not l.split():
                    comments += 1
                    continue
                idx = (n-comments) // 2
                if even:
                    centers[idx].append(np.array([float(i) for i in l.replace(",", "").split()]))
                    even = False
                else:
                    data[idx].append(np.array([float(i) for i in l.replace(",", "").split()]))
                    even = True

        # FIXME: this will lead to a subtly wrong error
        if idx is None:
            logging.warning("no data in '{}'".format(infile))
        elif idx + 1 < iterations:
            logging.warning("found only {}/{} iterations in '{}'".format(idx+1, iterations, infile))
            logging.warning("consider simulating longer, for now fill up with double results")
            n = idx + 1
            for i in range(idx + 1, iterations):
                centers[i].append(centers[i%n][-1])
                data[i].append(data[i%n][-1])

    # sort ranges by their minium center value
    centers, data = zip(*sorted(zip(centers, data), key=lambda x: np.min(x[0])))
    centers = list(centers)
    data = list(data)

    for i in range(iterations):
        centers[i] = np.array(centers[i])
        data[i] = np.array(data[i])

    if outformat is not None:
        outfile = outformat.format("wl_raw")
        with open(outfile, "w") as f:
            f.write("# S err count(S) count(S)_err\n")
            for ce, da in zip(centers, data):
                for c, d in zip(ce, da):
                    for l in zip(c, d):
                        f.write("{} {}\n".format(*l))
                    f.write("\n\n")

    return centers, data

def stitchInterpol(centers, data):
    """Calculate the 'Z' values needed to stitch parts of the distribution
    together. This is calculated from the overlap of two neighboring
    ranges. Warns if there is not enough overlap.

    Uses spline interpolation to measure 'Z'.

    Applies the 'Z' values to the data to generate a continuous
    distribution from the single parts.
    """
    for i in range(len(data)-1):
        try:
            spline_1 = interp1d(centers[i], data[i], kind='cubic')
        except ValueError as e:
            logging.error("too few samples: " + str(e))
            continue
        except TypeError as e:
            logging.error("too few samples: " + str(e))
            continue

        # calculate for every x the difference to y(x) - spline_1(x)
        Z = [y - spline_1(x) for x, y in zip(centers[i+1], data[i+1]) if min(centers[i]) < x < max(centers[i])]

        # not enough overlap
        if len(Z) < 3:
            logging.warning("not enough overlap ({}: {} -- {})".format(len(Z), max(centers[i]), min(centers[i+1])))

        z, err = bootstrap(Z)
        data[i+1] -= z


def histogramfragmentsToDistribution(centers, data, outformat=None):
    """Takes a list of lists, with histogram fragments.

    :param centers: [[centers of range 1], [of range 2], ...]
    :param data: [[counts of range 1], [of range 2], ...]
    """

    stitchInterpol(centers, data)

    if outformat is not None:
        outfile = outformat.format("wl_stiched")
        with open(outfile, "w") as f:
            f.write("# S count(S)\n")
            for c, d in zip(centers, data):
                for l in zip(c, d):
                    f.write("{} {}\n".format(*l))
                f.write("\n\n")

    # flatten
    centers = centers.flatten()
    data = data.flatten()

    # normalize
    mask = np.isfinite(data)
    mdata = data[mask]
    mcenters = centers[mask]

    data -= np.max(mdata)
    mdata -= np.max(mdata)
    area = trapz(np.exp(mdata), mcenters)
    data -= np.log(area)

    return centers, data


def run(parallelness=1):
    """Reads rawData files from a finished simulation, specified
    by 'parameters.py' in the same folder and evaluates it.
    """
    steps = param.parameters["number_of_steps"]
    d = param.parameters["rawData"]
    out = param.parameters["directory"]
    energies = param.parameters["energies"]

    outfiles = []
    means_file = "{}/means.dat".format(param.parameters["directory"])
    with open(means_file, "w") as f:
        f.write("# N mean/T err variance/T err\n")

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
        getMinMaxTime((f for f in names), parallelness)

        centers, data = readData(names, outbase)

        # centers should always be the same
        # dist is a matrix with one line per iteration and one column per bin
        dist = np.zeros([len(centers), len(centers[0][0])*len(centers[0])])
        dist_centers = np.zeros([len(centers), len(centers[0][0])*len(centers[0])])
        for n, m_d in zip(range(len(centers)), data):
            tmp_c, tmp_d = histogramfragmentsToDistribution(centers[n], m_d, outbase)
            dist[n] = tmp_d
            dist_centers[n] = tmp_c

        # componentwise mean and stderr over all distributions
        centers = centers[0].flatten()
        data = np.mean(dist, 0)
        stderr = np.std(dist, 0) / (len(dist) - 1)

        # save to file
        outfile = outbase.format("WL")
        with open(outfile, "w") as f:
            f.write("# S err ln(P(S)) ln(P(S)_err)\n")
            for i in zip(centers, data, stderr):
                f.write("{} nan {} {}\n".format(*i))

        # calculate mean and variance of the distribution
        sample = [getMeanFromDist(c, dat) for c, dat in zip(dist_centers, dist)]
        sample_v = [getVarFromDist(c, dat) for c, dat in zip(dist_centers, dist)]
        m = np.mean(sample)
        m_err = np.std(sample) / (len(dist) - 1)
        v = np.mean(sample_v)
        v_err = np.std(sample_v) / (len(dist) - 1)
        means_file = "{}/means.dat".format(param.parameters["directory"])
        with open(means_file, "a") as f:
            f.write("{} {} {} {} {}\n".format(N, m, m_err, v, v_err))

        normed_file = outbase.format("normed")
        with open(normed_file, "w") as f:
            f.write("# S err ln(P(S)) ln(P(S)_err)\n")
            for i in zip(centers, data, stderr):
                f.write("{} nan {} {}\n".format((i[0]-m)/v**0.5, np.exp(i[1])*v**0.5, np.exp(i[1])*i[2]))


if __name__ == "__main__":
    logging.info("started Wang Landau evaluation")
    run()
