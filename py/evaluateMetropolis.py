#!/usr/bin/env python3

import sys
from math import exp, log, ceil
import logging
from multiprocessing import Pool
import gzip

import numpy as np
from scipy.interpolate import interp1d
from scipy.integrate import simps, trapz

import parameters as param
from config import bootstrap, bootstrap_histogram, histogram_simple_error, SimulationInstance
from commonEvaluation import getMinMaxTime


logging.basicConfig(level=logging.INFO,
                format='%(asctime)s -- %(levelname)s :: %(message)s',
                datefmt='%d.%m.%YT%H:%M:%S')
logging.info("started")

proposedTheta = []

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
        for _ in range(15):
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


def averageOverSameX(whole_distribution):
    """Collects datapoints inside the same bins of different temperatures
    and averages over them. (must happen after stitching the distribution
    together)
    """
    xDict = {}
    for x, x_err, y, y_err in whole_distribution:
        if x not in xDict:
            xDict.update({x: []})
        xDict[x].append([x_err, y, y_err])

    out = []
    for x, l in xDict.items():
        l = list(zip(*l))
        mean_x_err = np.mean(l[0])
        mean_y = np.mean(l[1])
        mean_y_err = np.mean(l[2])
        out.append([x, mean_x_err, mean_y, mean_y_err])

    return out

def getDistribution(data, outfile, histfile, col, theta, steps, inBins=50):
    """reads samples from a file, outputs the distribution

    :param:data:        array of the samples (already purged from correlation)
    :param:outfile:     file to write the rescaled distribution to
    :param:histfile:    file to write the unrescaled distribution to
    :param:col:         in which column is the observable to be measured
    :param:theta:       at which "temperature" was the simulation done
    :param:steps:       how many steps are the random walks long
    :param:inBins:      bins to be used for the histogram
                        can be a number (how many bins)
                        or an iterable (borders of the bins)
    """
    # histograms do not need to be normed, this will happen in the end
    counts, bins = np.histogram(data, inBins)
    centers = (bins[1:] + bins[:-1])/2
    centers_err = [centers[i] - bins[i] for i in range(len(centers))]

    bs_mean, bs_err = bootstrap_histogram(data, bins=bins)
    #bs_err = histogram_simple_error(counts)

    data_p = []
    for s, s_err, pts, pts_err in zip(centers, centers_err, bs_mean, bs_err):
        #~ print(i, exp(i/theta) * j)
        try:
            ps_log = s/theta + log(pts)
            # gaussian erroro propagation
            ps_log_err = pts_err/pts  + s_err/theta
            if pts < 10:
                # ignore bin with too few entries
                raise ValueError
                pass
        except ValueError:
            pass
        else:
            data_p.append((s, s_err, ps_log, ps_log_err))

    logging.info(outfile)
    with open(outfile, "w") as f:
        f.write("# S S_err P(S) P(S)_err\n")
        for d in data_p:
            f.write("{} {} {} {}\n".format(*d))
    with open(histfile, "w") as f:
        f.write("# S S_err P(S) P(S)_err\n")
        for d in zip(centers, centers_err, bs_mean, bs_err):
            f.write("{} {} {} {}\n".format(*d))

    if not data_p:
        data_p.append([np.nan, np.nan, np.nan, np.nan])
        logging.info("no good data from T={}".format(theta))

    return data_p


def getZtheta(list_of_ps_log, thetas, outNames):
    """Calculate the 'Z' values needed to stitch parts of the distribution
    together. This is calculated from the overlap of two neighboring
    temperatures. Warns if there is not enough overlap.

    This version assumes that both temperatures use the same bins. (compare getZthetaInterpol())
    """
    # list_of_ps_log is a list in the form [[s1, s1_err, ps_log1, ps_log1_err], [s2, s2_err, ps_log2, ps_log2_err], ..]
    Ztheta_mean = [(0, 0)]
    for i in range(len(list_of_ps_log)-1):
        l1 = list_of_ps_log[i]
        l2 = list_of_ps_log[i+1]

        x1 = {x for x, _, _, _ in l1}
        y1 = {x: y for x, _, y, _ in l1}
        y1_err = {x: y_err for x, _, _, y_err in l1}
        x2, x2_err, y2, y2_err = zip(*l2)

        # calculate for every s in l2 the difference to l2(s) - l1(s)
        Z = [y - y1[x] for x, _, y, _ in l2 if x in x1]
        Z_err = [y_err + y1_err[x] for x, _, _, y_err in l2 if x in x1]

        # save S Z err to a file, to see later if there is a dependence on S (-> not equilibrated)
        with open(outNames[i], "w") as f:
            f.write("# s z err\n")
            for xyee, z, err in zip(l1, Z, Z_err):
                s = xyee[0]
                f.write("{} {} {}\n".format(s, z, err))

        # not enough overlap
        if len(Z) < 5:
            logging.warning("not enough overlap between {} and {}, insert an intermediate theta".format(thetas[i], thetas[i+1]))
            proposedTheta.append((thetas[i] + thetas[i+1]) / 2)

        Ztheta_mean.append(bootstrap(Z))

    return Ztheta_mean


def getZthetaInterpol(list_of_ps_log, thetas):
    """Calculate the 'Z' values needed to stitch parts of the distribution
    together. This is calculated from the overlap of two neighboring
    temperatures. Warns if there is not enough overlap.

    This version uses spline interpolation to measure 'Z' if the bins
    of the two temperatures are not the same. (compare getZtheta())
    """
    # list_of_ps_log is a list in the form [[s1, s1_err, ps_log1, ps_log1_err], [s2, s2_err, ps_log2, ps_log2_err], ..]
    Ztheta_mean = [(0, 0)]
    for i in range(len(list_of_ps_log)-1):
        l1 = list_of_ps_log[i]
        l2 = list_of_ps_log[i+1]

        # calculate a cubic spline through l1
        try:
            x, x_err, y, y_err = zip(*l1)
            spline_1 = interp1d(x, y, kind='cubic')
        except ValueError:
            logging.error("too few samples")
            Ztheta_mean.append((0,0))
            continue
        except TypeError:
            logging.error("too few samples")
            Ztheta_mean.append((0,0))
            continue

        # calculate for every s in l2 the difference to l2(s) - spline_l1(s)
        Z = [ps_log - spline_1(s) for s, _, ps_log, _ in l2 if min(x) < s < max(x)]

        # not enough overlap
        if len(Z) < 5:
            logging.warning("not enough overlap, insert an intermediate theta, eg. {}".format((thetas[i] + thetas[i+1]) / 2))
            proposedTheta.append((thetas[i] + thetas[i+1]) / 2)

        Ztheta_mean.append(bootstrap(Z))

    return Ztheta_mean


def stichFile(infile, outfile, z, dz):
    """Applies the 'Z' values to the data to generate a continuous
    distribution from the single parts.
    """
    data = []
    Dz = 0
    with open(infile, "r") as fin:
        with open(outfile, "w") as fout:
            fout.write("# S S_err P(S) P(S)_err\n")
            Dz += dz
            for line in fin.readlines():
                if line[0] == "#":
                    continue
                nums = list(map(float, line.split()))
                nums[2] -= z
                nums[3] += Dz
                data.append(nums)
                fout.write("{} {} {} {}\n".format(*nums))
    return data


def eval_simplesampling(name, outdir, N=0):
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

        s = "{} ".format(N)

        # r
        data = getDataFromFile(name, 3, float("inf"))
        s += "{} {} ".format(*bootstrap(data))
        s += "{} {} ".format(*bootstrap(data, f=np.var))

        # r2
        data = getDataFromFile(name, 4, float("inf"))
        s += "{} {} ".format(*bootstrap(data))
        s += "{} {} ".format(*bootstrap(data, f=np.var))

        # maxDiameter
        data = getDataFromFile(name, 5, float("inf"))
        s += "{} {} ".format(*bootstrap(data))
        s += "{} {} ".format(*bootstrap(data, f=np.var))

        # maxX
        data = getDataFromFile(name, 6, float("inf"))
        s += "{} {} ".format(*bootstrap(data))
        s += "{} {} ".format(*bootstrap(data, f=np.var))

        # maxY
        data = getDataFromFile(name, 7, float("inf"))
        s += "{} {} ".format(*bootstrap(data))
        s += "{} {} ".format(*bootstrap(data, f=np.var))
        s += "\n"

        with open("{}/simple.dat".format(outdir), "a") as f:
            f.write(s)


def run(histogram_type=1, parallelness):
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
    eval_simplesampling(None, out)

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
            eval_simplesampling(nameDict[float("inf")], out, N)

        # remove files from evaluation, which did not equilibrate
        not_aborted = []
        for T in theta_for_N:
            if not testIfAborted("{}/{}.dat".format(d, nameDict[T])):
                not_aborted.append(T)
            else:
                logging.info("not equilibrated: N={}, theta={}".format(N, T))
        theta_for_N = not_aborted

        # get some stats of the simulation and save it
        stats = getMinMaxTime("{}/{}.dat".format(d, n) for n in nameDict.values(), parallelness)

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
        #~ num_bins = ceil(num_samples**(1/2))
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

        with Pool(parallelness) as p:
            list_of_ps_log = p.starmap(getDistribution,
                                        [
                                            (
                                                dataDict[T],
                                                "{}/dist_{}.dat".format(out, nameDict[T]),
                                                "{}/hist_{}.dat".format(out, nameDict[T]),
                                                param.parameters["observable"],
                                                T,
                                                N,
                                                bins
                                            )
                                            for T in theta_for_N
                                        ]
                                      )

        for T in theta_for_N:
            outfiles.append('"{}/stiched_{}.dat"'.format(out, nameDict[T]))

        z = getZtheta(list_of_ps_log, theta_for_N, ["{}/Z_{}.dat".format(out, nameDict[T]) for T in theta_for_N])

        zc = [0]
        zce = [0]
        for i in z[1:]:
            zc.append(zc[-1] + i[0])
            zce.append(zce[-1] + i[1])

        with Pool(parallelness) as p:
            data = p.starmap(stichFile,
                               [("{}/dist_{}.dat".format(out, nameDict[T]),
                                 "{}/stiched_{}.dat".format(out, nameDict[T]),
                                 zc[n],
                                 zce[n]) for n, T in enumerate(theta_for_N)])
        whole_distribution = []
        for dat in data:
            whole_distribution += dat

        whole_distribution_file = param.basename.format(steps=N, **param.parameters)
        whole_distribution_file = "{}/whole_{}.dat".format(out, whole_distribution_file)

        # purge values with same x, by calculating mean
        # TODO: generate errors by bootstrapping
        whole_distribution = averageOverSameX(whole_distribution)

        # integrate, to get the normalization constant
        t = list(zip(*sorted(whole_distribution)))
        m = max(t[:][0])
        #~ area = simps(np.exp(t[2]), t[0])  # simpson is not robust against really small numbers :/
        area = trapz(np.exp(t[2]), t[0])
        with open(whole_distribution_file, "w") as f:
            f.write("# S S_err P(S) P(S)_err\n")
            for i in sorted(whole_distribution):
                f.write("{} {} {} {}\n".format(i[0], i[1], i[2]-log(area), i[3]))

    if proposedTheta:
        print("consider adding following thetas:")
        print(" ".join(map(str, proposedTheta)))


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
