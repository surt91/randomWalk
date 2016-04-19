#!/usr/bin/env python3

from math import exp, log, ceil
import logging
from multiprocessing import Pool

import numpy as np
from scipy.interpolate import interp1d
from scipy.integrate import simps, trapz

import parameters as param
from config import bootstrap, bootstrap_histogram, histogram_simple_error


logging.basicConfig(level=logging.INFO,
                format='%(asctime)s -- %(levelname)s :: %(message)s',
                datefmt='%d.%m.%YT%H:%M:%S')
logging.info("started")

proposedTheta = []


def getAutocorrTime(data):
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
    logging.info("autocorrelation time: " + str(tau))
    return tau


def getDataFromFile(filename, col):
    try:
        a = np.loadtxt(filename+".gz")
    except FileNotFoundError:
        try:
            a = np.loadtxt(filename)
        except FileNotFoundError:
            logging.error("cannot find " + filename)
            return

    data = a.transpose()[col]
    t_corr = getAutocorrTime(data)
    # do only keep statistically independent samples to not underestimate the error
    return data[::ceil(2*t_corr)]


def getStatistics(dataDict):
    count = 0
    minimum = 10**10
    maximum = 0
    for j in dataDict.values():
        count += len(j)
        minimum = min(minimum, np.min(j))
        maximum = max(maximum, np.max(j))

    return minimum, maximum, count


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

    return data_p


def getZtheta(list_of_ps_log, thetas):
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

        # not enough overlap
        if len(Z) < 5:
            logging.warning("not enough overlap, insert an intermediate theta")
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


def run():
    thetas = param.parameters["thetas"]
    steps = param.parameters["number_of_steps"]
    d = param.parameters["rawData"]
    out = param.parameters["directory"]

    outfiles = []

    column = param.parameters["observable"]

    for N in steps:
        list_of_ps_log = []
        try:
            theta_for_N = thetas[N]
        except KeyError:
            theta_for_N = thetas[0]

        # find names of needed files
        nameDict = {}
        for T in theta_for_N:
            nameDict.update({T: param.basename.format(steps=N,
                                                      theta=T,
                                                      **param.parameters
                                                 )
                            }
                           )

        with Pool() as p:
            tmp = p.starmap(getDataFromFile, [("{}/{}.dat".format(d, nameDict[T]), column) for T in theta_for_N])

        # load data
        dataDict = {theta_for_N[i]: tmp[i] for i in range(len(tmp))}

        # gather statistics over all data
        minimum, maximum, num_samples = getStatistics(dataDict)

        # https://en.wikipedia.org/wiki/Histogram#Number_of_bins_and_width
        # guess a good number of bins with rice rule
        num_bins = ceil(2*num_samples**(1/3))
        # in fact, I need more bins, guess them with the sqrt choice
        num_bins = ceil(num_samples**(1/2))
        # TODO: assign the bins adaptive: smaller bins where more data is

        bins = np.linspace(minimum, maximum, num=num_bins)

        for T in theta_for_N:
            data = getDistribution(dataDict[T],
                                   "{}/dist_{}.dat".format(out, nameDict[T]),
                                   "{}/hist_{}.dat".format(out, nameDict[T]),
                                   col=param.parameters["observable"],
                                   theta=T,
                                   steps=N,
                                   inBins=bins
                                  )
            list_of_ps_log.append(data)
            outfiles.append('"{}/stiched_{}.dat"'.format(out, nameDict[T]))

        z = getZtheta(list_of_ps_log, theta_for_N)

        zc = [0]
        zce = [0]
        for i in z[1:]:
            zc.append(zc[-1] + i[0])
            zce.append(zce[-1] + i[1])

        whole_distribution = []
        try:
            theta_for_N = thetas[N]
        except KeyError:
            theta_for_N = thetas[0]
        for n, T in enumerate(theta_for_N):
            name = param.basename.format(steps=N,
                                         theta=T,
                                         **param.parameters
                                         )
            data = stichFile("{}/dist_{}.dat".format(out, name),
                             "{}/stiched_{}.dat".format(out, name),
                             zc[n], zce[n])
            whole_distribution += data

        whole_distribution_file = param.basename.replace("_T{theta:.5f}", "").format(steps=N, **param.parameters)
        whole_distribution_file = "{}/whole_{}.dat".format(out, whole_distribution_file)

        # purge values with same x, by calculating mean
        # TODO: generate errors by bootstrapping
        whole_distribution = averageOverSameX(whole_distribution)

        # integrate, to get the normalization constant
        t = list(zip(*sorted(whole_distribution)))
        m = max(t[:][0])
        #~ area = simps(np.exp(t[2]), t[0])  # simpson is not robust against really small numbers :/
        area = trapz(np.exp(t[2]), t[0])
        print(area)
        with open(whole_distribution_file, "w") as f:
            f.write("# S S_err P(S) P(S)_err\n")
            for i in sorted(whole_distribution):
                f.write("{} {} {} {}\n".format(i[0], i[1], i[2]-log(area), i[3]))

    print("plot with gnuplot")
    print("p " + ", ".join(i + " u 1:3:2:4 w xyerr" for n, i in enumerate(outfiles)))
    print('p "{}" u 1:3:2:4 w xye'.format(whole_distribution_file))

    if proposedTheta:
        print("consider adding following thetas:")
        print(" ".join(map(str, set(proposedTheta))))


if __name__ == "__main__":
    run()
