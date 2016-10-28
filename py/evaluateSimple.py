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

proposedTheta = []


def getDataFromFile(filename, col):
    """Read timeseries data from a file.
    """
    data = np.loadtxt(filename+".gz").transpose()[col]
    logging.info("{} independent samples".format(len(data)))
    return data


def getStatistics(dataDict):
    """Collects fundamental statistics from a timeseries.
    """
    logging.info("collecting statistics")
    count = 0
    minimum = 10**10
    maximum = 0
    for j in dataDict.values():
        count += len(j)
        minimum = min(minimum, np.min(j))
        maximum = max(maximum, np.max(j))

    return minimum, maximum, count


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
            f.write("# N r err varR err r2 err varR2 err maxDiameter err varMaxDiameter err ... L err varL err A err varA err\n")
    else:
        name = "rawData/" + name + ".dat"
        # call getDataFromFile to purge it from correlated samples (by autocorrelationtime)
        # also, this saves RAM

        with Pool(parallelness) as p:
            data = p.starmap(getDataFromFile, [(name, i) for i in [3, 4, 5, 6, 7, 1, 2]])
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


def getWholeDistribution(data, bins, thetas, write_intermediate_files=False, out="", nameDict={}):
    """Given a dict of samples at different temperatures, return the
    distribution (combined and stichted).

    This is intendet to be used inside a bootstrap function and therefore
    does not calculate errors.
    If write_intermediate_files is true

    :param dataDict: dict{T: [data]} raw data to derive the distribution from
    :param bins: Number of bins or borders of bins to use in the histogram.
    :param thetas: sorted list of temperatures
    :returns: whole distribution
    """

    centers, centers_err = getCentersFromBins(bins)
    counts, bins = np.histogram(data, bins)
    counts = counts.astype(float)
    counts /= np.max(counts)

    # normalize to area of 1
    # integrate, to get the normalization constant
    area = trapz(counts, centers)
    counts -= log(area)

    return counts


def run(histogram_type=1, parallelness=1):
    """Reads rawData files from a finished simulation, specified
    by 'parameters.py' in the same folder and evaluates it.

    Does mainly generate a distribution.

    @param histogram_type can be: 1 for equispaced (i.e. linear)
                                  2 for logarithmic
                                  3 for percentile based (i.e. flat)
    """
    thetas = param.parameters["thetas"]
    steps = param.parameters["number_of_steps"]
    d = param.parameters["rawData"]
    out = param.parameters["directory"]
    sampling = param.parameters["sampling"]
    t_eq = param.parameters["t_eq"]

    outfiles = []

    # prepare file
    eval_simplesampling(None, out, parallelness=parallelness)
    means_file = "{}/means.dat".format(out)
    with open(means_file, "w") as f:
        f.write("# N mean/T err variance/T err\n")

    column = param.parameters["observable"]
    if column == 3:
        column = 1

    for N in steps:
        logging.info("N = {}".format(N))

        # find names of needed files
        nameDict = {}
        T = float("inf")
        theta_for_N = [T]
        i = SimulationInstance(steps=N, theta=T, **param.parameters)
        nameDict.update({T: i.basename})

        # evaluate things from simple sampling (mainly for literature comparison)
        eval_simplesampling(nameDict[float("inf")], out, N, parallelness=parallelness)

        # get some stats of the simulation and save it
        stats = getMinMaxTime(("{}/{}.dat".format(d, n) for n in nameDict.values()), parallelness=parallelness)

        with open("{}/stats_N{}.dat".format(out, N), "w") as f:
            f.write("# theta time/sweep vmem MC_tries MC_rejects\n")
            for theta, times, mems, versions, tries, reject in sorted(zip(*stats)):
                f.write("{} {} {} {} {}\n".format(theta, times, mems, tries, reject))


        # read all the files -- in parallel (also adjust for autocorrelation)
        with Pool(parallelness) as p:
            tmp = p.starmap(getDataFromFile, [("{}/{}.dat".format(d, nameDict[T]), column) for T in theta_for_N])

        # load data
        dataDict = {theta_for_N[i]: tmp[i] for i in range(len(tmp))}

        # gather statistics over all data
        minimum, maximum, num_samples = getStatistics(dataDict)

        # https://en.wikipedia.org/wiki/Histogram#Number_of_bins_and_width
        # guess a good number of bins with rice rule
        num_bins = ceil(2*num_samples**(1/3))
        # in fact, I need more bins, guess them with the sqrt choice
        # but ensure that we only get max 1 bin per 2 x-axis values
        # since sometimes they are discrete, which can result in artifacts
        num_bins = min(num_bins, (maximum - minimum))
        num_bins = max(num_bins, 1)

        logging.info("using {} bins".format(num_bins))

        # get bins, according to the chosen type
        if histogram_type == 1:
            bins = np.linspace(minimum, maximum, num=num_bins)
        elif histogram_type == 2:
            bins = np.logspace(np.log10(minimum), np.log10(maximum), num=num_bins)
        elif histogram_type == 3:
            bins = getPercentileBasedBins(dataDict, num_bins)
        else:
            raise

        # to create intermediate files (but without errors)
        centers, centers_err = getCentersFromBins(bins)
        getWholeDistribution(dataDict[float("inf")], bins, theta_for_N, True, out, nameDict)

if __name__ == "__main__":
    logging.info("started Metropolis evaluation")
    # decide which histogram type to use:
    # equi spaced histograms seem to be better for Gaussian walks
    # percentile based, flat histograms, lead to similar errors for all bins
    if "--lin" in sys.argv:
        ht = 1
        logging.info("Using equi-spaced histogram")
    elif "--log" in sys.argv:
        ht = 2
        logging.info("Using logarithmic histogram")
    elif "--flat":
        ht = 3
        logging.info("Using percentile-based histogram")
    else:
        raise

    run(ht)
