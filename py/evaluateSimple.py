#!/usr/bin/env python3

import os
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
from scipy.optimize import curve_fit
import matplotlib
matplotlib.use('Agg')
from matplotlib import pyplot as plt

import parameters as param
from data import nu as nu_dict
from config import bootstrap, SimulationInstance
from commonEvaluation import getMinMaxTime, getMeanFromDist, getVarFromDist


logging.basicConfig(level=logging.INFO,
                format='%(asctime)s -- %(levelname)s :: %(message)s',
                datefmt='%d.%m.%YT%H:%M:%S')
logging.info("started")


def powerlaw(x, a, b, c):
    return a*x**b + c


def asymptoteFromFit(N, S, err=None, p0=(-2, -0.5, 1), outname=None):
    popt, pcov = curve_fit(powerlaw, N, S, p0, sigma=err)
    if outname is not None:
        plt.clf()
        lin = np.linspace(min(N), max(N), num=1000)
        plt.plot(lin, powerlaw(lin, *popt), 'b')
        plt.plot(N, S)
        plt.savefig(outname)
    return popt[2]


def asymptoticMeans(Ns, filenames):
    d = param.parameters["dimension"]
    nu = nu_dict[param.parameters["typ"]][d]

    n_resample = 200
    Lc = []
    Ac = []
    data = {}
    for N in Ns:
        name = filenames[N]
        name = "rawData/" + name + ".dat"
        data[N] = np.loadtxt(name+".gz").transpose()[1:3]

    length = len(data[N][0])

    for i in range(n_resample):
        L = []
        Lerr = []
        A = []
        Aerr = []
        for N in Ns:
            Lsample = np.random.choice(data[N][0]/N**((d-1)*nu), length, replace=True)
            L.append(np.mean(Lsample))
            Lerr.append(np.std(Lsample))
            Asample = np.random.choice(data[N][1]/N**((d)*nu), length, replace=True)
            A.append(np.mean(Asample))
            Aerr.append(np.std(Asample))

        Lc.append(asymptoteFromFit(Ns, L, err=Lerr))
        Ac.append(asymptoteFromFit(Ns, A, err=Aerr))

    asymptoteFromFit(Ns, L, outname="Ltest.png")
    asymptoteFromFit(Ns, A, outname="Atest.png")

    print("L:", np.mean(Lc), np.std(Lc))
    print("A:", np.mean(Ac), np.std(Ac))


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

        counts, bins = getDistribution(data[0])
        centers, _ = getCentersFromBins(bins)
        with open("{}/distL_N{}.dat".format(outdir, N), "w") as f:
            for c, d in zip(centers, counts):
                f.write("{} {}\n".format(c, d))

        counts, bins = getDistribution(data[1])
        centers, _ = getCentersFromBins(bins)
        with open("{}/distA_N{}.dat".format(outdir, N), "w") as f:
            for c, d in zip(centers, counts):
                f.write("{} {}\n".format(c, d))


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
    """

    if bins is None:
        bins = np.max(data) - np.min(data)
        while bins > 200:
            bins /= 2
        bins = int(bins)

    counts, bins = np.histogram(data, bins)
    centers, centers_err = getCentersFromBins(bins)
    counts = counts.astype(float)
    counts /= np.max(counts)

    # normalize to area of 1
    # integrate, to get the normalization constant
    area = trapz(counts, centers)
    counts /= area

    return counts, bins


def cat(out, in_list):
    with gzip.open(out+".gz", 'w') as outfile:
        for fname in in_list:
            with gzip.open(fname+".gz") as infile:
                for line in infile:
                    outfile.write(line)


def run(parallelness=1):
    steps = param.parameters["number_of_steps"]
    out = param.parameters["directory"]
    num_batches = param.parameters["batches"]
    iterations = param.parameters["iterations"]
    del param.parameters["iterations"]

    # prepare file
    eval_simplesampling(None, out, parallelness=parallelness)

    # merge batches and collect files to evaluate
    filenames = {}
    for N in steps:
        to_merge = []
        tmp = SimulationInstance(steps=N, batch_id=-1, iterations=iterations, **param.parameters).basename
        # if the output does not already exist, generate it from the batches
        if not os.path.isfile("rawData/" + tmp + ".dat.gz"):
            logging.info("merge for N = {}".format(N))
            for i in range(num_batches):
                batch_iterations = iterations / num_batches
                if i == num_batches - 1:
                    batch_iterations += iterations % num_batches
                name = SimulationInstance(steps=N, batch_id=i, iterations=batch_iterations, **param.parameters).basename
                to_merge.append("rawData/" + name + ".dat")
            cat("rawData/" + tmp + ".dat", to_merge)
        filenames.update({N: tmp})

    for N in steps:
        logging.info("N = {}".format(N))

        name = filenames[N]

        # evaluate things from simple sampling (mainly for literature comparison)
        eval_simplesampling(name, out, N, parallelness=parallelness)


if __name__ == "__main__":
    logging.info("started Simple Sampling evaluation")

    run()
