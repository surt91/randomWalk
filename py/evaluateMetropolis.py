#!/usr/bin/env python3

import sys
from math import exp, log, ceil
import logging
from multiprocessing import Pool
import gzip

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

def bs_wrapper(tup):
    i, xRaw, f, kwargs = tup
    np.random.seed(i)
    newDict = {k: np.random.choice(v, len(v), replace=True) for k, v in xRaw.items()}
    return f(newDict, **kwargs)

def bootstrap_metropolis_distributions(xRaw, N, f=np.histogram, n_resample=100, parallelness=1, centers=None, **kwargs):
    """Bootstrap resampling, reduction function takes a list and returns
    a list of len N. Returns a list of means and a list of errors.
    6 return values:
        0,1 : distribution, err
        2,3 : mean, err (i.e. int x p(x) dx)
        4,5 : var, err (i.e. int x**2 p(x) dx - mu**2)

    :param xRaw:    vector of raw input data
    :param N:       length of list returned by f
    :param f:       reduction function, takes a list, returns a list
    :param kwargs:  keyword arguments for f
    """
    if not len(xRaw):
        return float("NaN"), float("NaN")

    # do the bootstrapping in parallel, if parallelness is given
    with Pool(parallelness) as p:
        counts = p.map(bs_wrapper, [(i, xRaw, f, kwargs) for i in range(n_resample)])

    # copy results to np.array
    allCounts = np.zeros((n_resample, N), dtype=np.float)
    means = np.zeros((n_resample), dtype=np.float)
    var = np.zeros((n_resample), dtype=np.float)
    for n, i in enumerate(counts):
        allCounts[n] = i

        # kill nan from data
        c = centers[np.isfinite(i)]
        i = i[np.isfinite(i)]
        means[n] = getMeanFromDist(c, i)
        var[n] = getVarFromDist(c, i)

    return (
            np.mean(allCounts, 0), np.std(allCounts, 0),
            np.mean(means), np.std(means),
            np.mean(var), np.std(var),
           )

# http://stackoverflow.com/a/16045141/1698412
def autocorrelation(x):
    """
    Compute autocorrelation using FFT
    The idea comes from
    http://dsp.stackexchange.com/a/1923/4363 (Hilmar)
    """
    x = np.asarray(x)
    N = len(x)
    x = x-x.mean()
    s = np.fft.fft(x, N*2-1)
    result = np.real(np.fft.ifft(s * np.conjugate(s), N*2-1))
    result = result[:N]
    result /= result[0]
    return result


def getAutocorrTime(data, T="?"):
    """Calculates the autocorrelation time of a time series.

    :param:data: timeseries

    returns autocorrelation time
    """
    #~ autocorr = np.correlate(data-np.mean(data), data-np.mean(data), mode='full')[len(data)-1:]
    autocorr = autocorrelation(data)
    x0 = autocorr[0]

    # integrate only up to the first negative values
    m = -1
    for n, i in enumerate(autocorr):
        if i < 0:
            m = n
            break

    tau = sum(autocorr[:m])/x0
    logging.info("theta = {}, t_corr: {:.1f}".format(T, tau))
    return tau


def testIfAborted(filename):
    """Tests if the simulation was aborted.
    This is the case if it did not equilibrate.
    """
    with gzip.open(filename+".gz") as f:
        # it will be noted in the first 15 lines, if we aborted
        for _ in range(25):
            if b"# Does not equilibrate" in f.readline():
                return True
    return False


def getDataFromFile(filename, col, T="?", t_eq=None):
    """Read timeseries data from a file, and return a subset of that
    data purged from correlation.
    """
    # read first few lines to determine the autocorrelation
    t_corr = float("inf")
    num = 10**2
    # repeat until autocorrelation time is far smaller than samples used
    # to determine it
    while t_corr > num/5:
        num *= 10
        mc_time = [0] * num
        S = [0] * num

        comment = 0
        start = 0
        end = num
        if t_eq:
            start += 2*t_eq
            end += 2*t_eq

        try:
            with gzip.open(filename+".gz", "rt") as f:
                for n, line in enumerate(f):
                    if "#" in line or not line.strip():
                        comment += 1
                        continue
                    if n-comment >= num:
                        break
                    data = tuple(map(float, line.split()))
                    #~ print(n, comment, end, num, n-comment, len(mc_time), len(S), col, data)
                    mc_time[n-comment] = data[0]
                    S[n-comment] = data[col]
        except FileNotFoundError:
            logging.error("cannot find " + filename)
            return

        t_corr = getAutocorrTime(S, T=T)

    # do only keep statistically independent samples to not underestimate the error
    data = []
    with gzip.open(filename+".gz", "rt") as f:
        next_to_read = start
        for n, line in enumerate(f):
            # ignore empty lines and comment lines
            if "#" in line or not line.strip():
                comment += 1
                continue
            if n-comment == next_to_read:
                next_to_read += ceil(2*t_corr)
                try:
                    d = tuple(map(float, line.split()))[col]
                except:
                    logging.error("can somehow not read col {} at line {}: ('{}')".format(col, n, line)),
                    d = float("nan")
                data.append(d)

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
            f.write("# N r err varR err r2 err varR2 err maxDiameter err varMaxDiameter err ... \n")
    else:
        name = "rawData/" + name + ".dat"
        # call getDataFromFile to purge it from correlated samples (by autocorrelationtime)
        # also, this saves RAM

        with Pool(parallelness) as p:
            data = p.starmap(getDataFromFile, [(name, i, float("inf")) for i in [3, 4, 5, 6, 7]])
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


def getZ(l1, l2, T1="?", T2="?"):
    """Find the value Z to subtract from l2, such that it coincides with
    l1 (in their overlapping region).

    l1 and l2 are coordinate tuples, i.e., l2 = ((s1, p1), (s2, p2), ...)
    """
    # calculate for every s in l2 the difference to l2(s) - l1(s)
    x1 = {x for x, _ in l1}
    y1 = {x: y for x, y in l1}
    Z = [y - y1[x] for x, y in l2 if x in x1]

    #~ if len(Z) < 5:
        #~ logging.warning("not enough overlap between {} and {}, insert an intermediate theta".format(T1, T2))

    with warnings.catch_warnings():
        warnings.simplefilter("ignore", category=RuntimeWarning)
        ret = np.nanmean(Z)

    return ret


def getWholeDistribution(dataDict, bins, thetas, write_intermediate_files=False, out="", nameDict={}):
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

    ps_log_list = []
    center_list = []
    centers, centers_err = getCentersFromBins(bins)
    Z = []
    for n, T in enumerate(thetas):
        data = dataDict[T]
        counts, bins = np.histogram(data, bins)

        data_p = []
        tmp_center = []
        for m, s, pts in zip(range(len(centers)), centers, counts):
            try:
                ps_log = s/T + log(pts)
                if pts < 10:
                    # ignore bin with too few entries
                    raise ValueError
                    pass
            except ValueError:
                pass
            else:
                data_p.append(ps_log)
                tmp_center.append(s)

        if not data_p:
            data_p.append(np.nan)
            tmp_center.append(np.nan)
            logging.info("no good data from T={}".format(T))

        ps_log_list.append(np.array(data_p))
        center_list.append(np.array(tmp_center))

        try:
            l1 = tuple(zip(center_list[-2], ps_log_list[-2]))
            l2 = tuple(zip(center_list[-1], ps_log_list[-1]))
        except IndexError:
            # in the first iteration, this will fail, but that is ok
            pass
        else:
            Z.append(getZ(l1, l2, thetas[n-1], T))

        if write_intermediate_files:
            distfile = "{}/dist_{}.dat".format(out, nameDict[T])
            histfile = "{}/hist_{}.dat".format(out, nameDict[T])
            #~ logging.info(distfile)
            with open(distfile, "w") as f:
                f.write("# S S_err P(S) P(S)_err\n")
                for d in zip(tmp_center, data_p):
                    f.write("{} nan {} nan\n".format(*d))
            #~ logging.info(histfile)
            with open(histfile, "w") as f:
                f.write("# S S_err P(S) P(S)_err\n")
                for d in zip(centers, counts):
                    f.write("{} nan {} nan\n".format(*d))

    # cumulative offset (first span has an offset of 0)
    zc = [0]
    for i in Z:
        zc.append(zc[-1] + i)

    # stitch the single distributions together using zc
    for i in range(len(zc)):
        ps_log_list[i] -= zc[i]

    if write_intermediate_files:
        for n, T in enumerate(thetas):
            stitchfile = "{}/stitch_{}.dat".format(out, nameDict[T])
            c = center_list[n]
            counts = ps_log_list[n]
            #~ logging.info(stitchfile)
            with open(stitchfile, "w") as f:
                f.write("# S S_err P(S) P(S)_err\n")
                for d in zip(c, counts):
                    f.write("{} nan {} nan\n".format(*d))

    # flatten list, and average entries in the same bin
    center_array = np.concatenate(center_list, axis=0)
    ps_log_array = np.concatenate(ps_log_list, axis=0)
    s = np.array(sorted(set(centers)))
    p = np.zeros(len(s))

    with warnings.catch_warnings():
        warnings.simplefilter("ignore", category=RuntimeWarning)
        for n, i in enumerate(s):
            p[n] = np.nanmean(ps_log_array[center_array == i])

    # filter nans (i.e. bin with no entries)
    mask = np.isfinite(p)
    s_for_int = s[mask]
    p_for_int = p[mask]

    # normalize to area of 1
    # integrate, to get the normalization constant
    area = trapz(np.exp(p_for_int), s_for_int)
    p -= log(area)

    return p


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

    for N in steps:
        logging.info("N = {}".format(N))

        try:
            theta_for_N = thetas[N]
        except KeyError:
            theta_for_N = thetas[0]

        # fill the undefined entries with None
        try:
            t_eq[N]
        except KeyError:
            t_eq[N] = {T: None for T in theta_for_N}
        else:
            for T in theta_for_N:
                try:
                    t_eq[N][T]
                except KeyError:
                    t_eq[N][T] = None

        # find names of needed files
        nameDict = {}
        if sampling == 1:
            for T in theta_for_N:
                i = SimulationInstance(steps=N, theta=T, **param.parameters)
                nameDict.update({T: i.basename})
        elif sampling == 4:
            i = SimulationInstance(steps=N, theta=theta_for_N, **param.parameters)
            for T, bn in zip(i.T, i.basename):
                nameDict.update({T: bn})
        else:
            logging.error("unkown sampling method")

        # evaluate things from simple sampling (mainly for literature comparison)
        if float("inf") in theta_for_N:
            eval_simplesampling(nameDict[float("inf")], out, N, parallelness=parallelness)

        # remove files from evaluation, which did not equilibrate
        not_aborted = []
        for T in theta_for_N:
            if not testIfAborted("{}/{}.dat".format(d, nameDict[T])):
                not_aborted.append(T)
            else:
                logging.info("not equilibrated: N={}, theta={}".format(N, T))
        theta_for_N = not_aborted

        # get some stats of the simulation and save it
        stats = getMinMaxTime(("{}/{}.dat".format(d, n) for n in nameDict.values()), parallelness=parallelness)

        with open("{}/stats_N{}.dat".format(out, N), "w") as f:
            f.write("# theta time/sweep vmem MC_tries MC_rejects\n")
            for theta, times, mems, versions, tries, reject in sorted(zip(*stats)):
                f.write("{} {} {} {} {}\n".format(theta, times, mems, tries, reject))


        # read all the files -- in parallel (also adjust for autocorrelation)
        with Pool(parallelness) as p:
            tmp = p.starmap(getDataFromFile, [("{}/{}.dat".format(d, nameDict[T]), column, T, t_eq[N][T]) for T in theta_for_N])

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
        getWholeDistribution(dataDict, bins, theta_for_N, True, out, nameDict)

        # estimate the whole distribution with errors
        dist, err, mean, m_err, var, v_err = bootstrap_metropolis_distributions(
                            dataDict,
                            N=len(bins)-1,
                            f=getWholeDistribution,
                            bins=bins,
                            thetas=theta_for_N,
                            parallelness=parallelness,
                            centers=centers
                    )

        whole_distribution_file = param.basename.format(steps=N, **param.parameters)
        whole_distribution_file = "{}/whole_{}.dat".format(out, whole_distribution_file)
        with open(whole_distribution_file, "w") as f:
            f.write("# S S_err P(S) P(S)_err\n")
            for data in zip(centers, centers_err, dist, err):
                f.write("{} {} {} {}\n".format(*data))

        # TODO get errors by bootstrapping
        with open(means_file, "a") as f:
            f.write("{} {} {} {} {}\n".format(N, mean, m_err, var, v_err))


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
