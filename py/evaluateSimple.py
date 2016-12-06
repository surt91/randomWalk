#!/usr/bin/env python3

import sys
from math import exp, log, ceil
import logging
from multiprocessing import Pool
import gzip
import math

import numpy as np
import warnings
from scipy.interpolate import interp1d
from scipy.integrate import simps, trapz

import parameters as param
from config import bootstrap, SimulationInstance
from commonEvaluation import getMinMaxTime, getMeanFromDist, getVarFromDist


logging.basicConfig(level=logging.INFO,
                format='%(asctime)s -- %(levelname)s :: %(message)s',
                datefmt='%d.%m.%YT%H:%M:%S')
logging.info("started")


def getPercentileBasedBins(dataDict, numBins=100):
    """Generate histogram bins based on 'percentiles'.
    This leads to a flat histogram.
    """
    logging.info("determining nice bins for flat histogram")
    rawData = np.concatenate(list(dataDict.values()))

    # numBins bins, defined by their numBins+1 edges
    percentiles = np.linspace(0, 100, numBins+1)
    tmp = np.percentile(rawData, percentiles)

    # ensure max 1 bin per 3 unit scale
    right = 0
    bins = [0]
    for i in tmp:
        if ceil(i) - 2 <= right:
            continue
        right = ceil(i)
        bins.append(right)

    return bins


def eval_simplesampling(name, outdir, N=0, parallelness=1):
    """Evaluates some fundamental observables from simple sampling
    (i.e. theta = inf)
    """
    if name is None:
        with open("{}/simple.dat".format(outdir), "w") as f:
            f.write("# N L err varL err A err varA err r err varR err r2 err varR2 err maxDiameter err varMaxDiameter err ... \n")
    else:
        name = "rawData/" + name + ".dat"
        data = np.loadtxt(name+".gz").transpose()[1:]  # exclude the number of the sample

        with Pool(parallelness) as p:
            bs_mean = p.starmap(bootstrap, [(d, np.mean) for d in data])
            bs_var = p.starmap(bootstrap, [(d, np.var) for d in data])

        s = "{} ".format(N)

        for m, v in zip(bs_mean, bs_var):
            s += "{} {} ".format(*m)
            s += "{} {} ".format(*v)

        s += "\n"

        with open("{}/simple.dat".format(outdir), "a") as f:
            f.write(s)


def getCentersFromBins(bins):
    """Calculates the centers of the bins.

    :param bins: Borders of bins.
    """
    try:
        iterator = iter(bins)
    except TypeError:
        logging.error("bins needs to be an iterable")
        sys.exit(1)

    centers = (bins[1:] + bins[:-1])/2
    centers_err = [centers[i] - bins[i] for i in range(len(centers))]

    return centers, centers_err


def getDistribution(data, bins=None):
    """Given a dict of samples at different temperatures, return the
    distribution (combined and stichted).

    This is intendet to be used inside a bootstrap function and therefore
    does not calculate errors.
    If write_intermediate_files is true

    :param dataDict: dict{T: [data]} raw data to derive the distribution from
    :param bins: Number of bins or borders of bins to use in the histogram.
    :returns: whole distribution
    """

    counts, bins = np.histogram(data, bins)
    centers, centers_err = getCentersFromBins(bins)
    counts = counts.astype(float)
    counts /= np.max(counts)

    # normalize to area of 1
    # integrate, to get the normalization constant
    area = trapz(counts, centers)
    counts -= log(area)

    return counts


def run(parallelness=1):
    steps = param.parameters["number_of_steps"]
    d = param.parameters["rawData"]
    out = param.parameters["directory"]
    sampling = param.parameters["sampling"]

    outfiles = []

    # prepare file
    eval_simplesampling(None, out, parallelness=parallelness)

    for N in steps:
        logging.info("N = {}".format(N))

        # find names of needed files
        i = SimulationInstance(steps=N, theta=float("inf"), **param.parameters)
        name = i.basename

        # evaluate things from simple sampling (mainly for literature comparison)
        eval_simplesampling(name, out, N, parallelness=parallelness)


if __name__ == "__main__":
    logging.info("started Simple Sampling evaluation")

    run()
