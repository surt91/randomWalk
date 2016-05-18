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


logging.basicConfig(level=logging.INFO,
                format='%(asctime)s -- %(levelname)s :: %(message)s',
                datefmt='%d.%m.%YT%H:%M:%S')
logging.info("started")

proposedTheta = []


def getAutocorrTime(data, T="?"):
    # just take the first 5000, should be sufficient
    data = data[:5000]
    autocorr = np.correlate(data-np.mean(data), data-np.mean(data), mode='full')[len(data)-1:]
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


def getMinMaxTime(filenames):
    times = []
    mems = []
    for filename in filenames:
        with gzip.open(filename+".gz", "rt") as f:
            for i in f.readlines():
                if "# Does not equilibrate" in i:
                    break
                if "# time in seconds" in i or "# time/sweep in seconds" in i:
                    s = i.split(":")[-1].strip().strip("s")
                    times.append(float(s))
                if "# max vmem: VmPeak" in i:
                    s = i.split(":")[-1].strip().strip(" kB")
                    mems.append(float(s))
    try:
        logging.info("time/sweep between {:.2f}s - {:.2f}s".format(min(times), max(times)+0.0051))
    except ValueError:
        logging.info("No time measured")
    try:
        logging.info("memory between {:.0f}kB - {:.0f}kB".format(min(mems), max(mems)))
    except ValueError:
        logging.info("No memory measured")


def testIfAborted(filename):
    with gzip.open(filename+".gz") as f:
        # it will be noted in the first 5 lines, if we aborted
        for _ in range(5):
            if b"# Does not equilibrate" in f.readline():
                return True
    return False


def getDataFromFile(filename, col, T="?"):
    try:
        a = np.loadtxt(filename+".gz")
    except FileNotFoundError:
        try:
            a = np.loadtxt(filename)
        except FileNotFoundError:
            logging.error("cannot find " + filename)
            return

    data = a.transpose()[col]
    t_corr = getAutocorrTime(data, T=T)
    # do only keep statistically independent samples to not underestimate the error
    return data[::ceil(2*t_corr)]


def getStatistics(dataDict):
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
    data = []
    with open(infile, "r") as fin:
        with open(outfile, "w") as fout:
            fout.write("# S S_err P(S) P(S)_err\n")
            for line in fin.readlines():
                if line[0] == "#":
                    continue
                nums = list(map(float, line.split()))
                nums[2] -= z
                nums[3] += dz
                data.append(nums)
                fout.write("{} {} {} {}\n".format(*nums))
    return data


def eval_simplesampling(name, outdir, N=0):
    if name is None:
        with open("{}/simple.dat".format(outdir), "w") as f:
            f.write("# N r err varR err r2 err varR2 err maxDiameter err varMaxDiameter err ... \n")
    else:
        a = np.loadtxt("rawData/" + name + ".dat.gz")
        a = a.transpose()
        r = a[3]
        r2 = a[4]
        maxDiameter = a[5]
        maxX = a[6]
        maxY = a[7]

        with open("{}/simple.dat".format(outdir), "a") as f:
            s = "{} ".format(N)
            s += "{} {} ".format(*bootstrap(r))
            s += "{} {} ".format(*bootstrap(r, f=np.var))
            s += "{} {} ".format(*bootstrap(r2))
            s += "{} {} ".format(*bootstrap(r2, f=np.var))
            s += "{} {} ".format(*bootstrap(maxDiameter))
            s += "{} {} ".format(*bootstrap(maxDiameter, f=np.var))
            s += "{} {} ".format(*bootstrap(maxX))
            s += "{} {} ".format(*bootstrap(maxX, f=np.var))
            s += "{} {} ".format(*bootstrap(maxY))
            s += "{} {} ".format(*bootstrap(maxY, f=np.var))
            s += "\n"

            f.write(s)


def run(flatHistogram=True):
    thetas = param.parameters["thetas"]
    steps = param.parameters["number_of_steps"]
    d = param.parameters["rawData"]
    out = param.parameters["directory"]

    outfiles = []

    # prepare file
    eval_simplesampling(None, out)

    column = param.parameters["observable"]

    for N in steps:
        logging.info("N = {}".format(N))

        list_of_ps_log = []
        try:
            theta_for_N = thetas[N]
        except KeyError:
            theta_for_N = thetas[0]

        # find names of needed files
        nameDict = {}
        for T in theta_for_N:
            i = SimulationInstance(steps=N, theta=T, **param.parameters)
            nameDict.update({T: i.basename})

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

        getMinMaxTime("{}/{}.dat".format(d, n) for n in nameDict.values())

        with Pool() as p:
            tmp = p.starmap(getDataFromFile, [("{}/{}.dat".format(d, nameDict[T]), column, T) for T in theta_for_N])

        # load data
        dataDict = {theta_for_N[i]: tmp[i] for i in range(len(tmp))}

        # gather statistics over all data
        minimum, maximum, num_samples = getStatistics(dataDict)

        # https://en.wikipedia.org/wiki/Histogram#Number_of_bins_and_width
        # guess a good number of bins with rice rule
        num_bins = ceil(2*num_samples**(1/3))
        # in fact, I need more bins, guess them with the sqrt choice
        num_bins = ceil(num_samples**(1/2))
        # but ensure that we only get max 1 bin per 2 x-axis values
        # since sometimes they are discrete, which can result in artifacts
        num_bins = min(num_bins, (maximum - minimum))

        bins = np.linspace(minimum, maximum, num=num_bins)

        # get bins, such that every bin contains roughly the same amount
        # of data points (total)
        if flatHistogram:
            bins = getPercentileBasedBins(dataDict, num_bins)

        for T in theta_for_N:
            data = getDistribution(dataDict[T],
                                   "{}/dist_{}.dat".format(out, nameDict[T]),
                                   "{}/hist_{}.dat".format(out, nameDict[T]),
                                   col=param.parameters["observable"],
                                   theta=T,
                                   steps=N,
                                   inBins=bins
                                  )
            # only append, if we got some good data from the file
            list_of_ps_log.append(data)
            outfiles.append('"{}/stiched_{}.dat"'.format(out, nameDict[T]))

        z = getZtheta(list_of_ps_log, theta_for_N, ["{}/Z_{}.dat".format(out, nameDict[T]) for T in theta_for_N])

        zc = [0]
        zce = [0]
        for i in z[1:]:
            zc.append(zc[-1] + i[0])
            zce.append(zce[-1] + i[1])

        whole_distribution = []
        for n, T in enumerate(theta_for_N):
            data = stichFile("{}/dist_{}.dat".format(out, nameDict[T]),
                             "{}/stiched_{}.dat".format(out, nameDict[T]),
                             zc[n], zce[n])
            whole_distribution += data

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

    print("plot with gnuplot")
    print("p " + ", ".join(i + " u 1:3:2:4 w xyerr" for n, i in enumerate(outfiles)))
    print('p "{}" u 1:3:2:4 w xye'.format(whole_distribution_file))

    if proposedTheta:
        print("consider adding following thetas:")
        print(" ".join(map(str, proposedTheta)))


if __name__ == "__main__":
    logging.info("started Metropolis evaluation")
    # decide which histogram type to use:
    # equi spaced histograms seem to be better for Gaussian walks
    # percentile based, flat histograms, lead to similar errors for all bins
    if "--noFlat" in sys.argv:
        flatHistogram = False
        logging.info("Using equi-spaced histogram")
    else:
        logging.info("Using percentile-based histogram")
        flatHistogram = True

    run(flatHistogram)
