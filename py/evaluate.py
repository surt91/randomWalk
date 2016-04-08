#!/usr/bin/env python3

from math import exp, log, ceil
import logging

import numpy as np
from scipy.interpolate import interp1d
from scipy.integrate import simps, trapz

import parameters as param
from config import bootstrap, bootstrap_histogram, histogram_simple_error


logging.basicConfig(level=logging.INFO,
                format='%(asctime)s -- %(levelname)s :: %(message)s',
                datefmt='%d.%m.%YT%H:%M:%S')
logging.info("started")


def getAutocorrTime(data):
    # just take the first 5000, should be sufficient
    data = data[:5000]
    autocorr = np.correlate(data-np.mean(data), data-np.mean(data), mode='full')[len(data)-1:]
    x0 = autocorr[0]

    #~ # integrate only up to the first negative values
    m = -1
    for n, i in enumerate(autocorr):
        if i < 0:
            m = n
            break

    tau = sum(autocorr[:m])/x0
    logging.info("autocorrelation time: " + str(tau))
    return tau

def getDistribution(infile, outfile, histfile, theta, steps, num_bins=50):
    try:
        a = np.loadtxt(infile+".gz")
    except FileNotFoundError:
        try:
            a = np.loadtxt(infile)
        except FileNotFoundError:
            logging.error("cannot find " + self.filename)
            return

    col = param.parameters["observable"]

    data = a.transpose()[col]
    t_corr = getAutocorrTime(data)
    # do only keep statistically independent samples to not underestimate the error
    data = data[::ceil(2*t_corr)]


    # histograms do not need to be normed, this will happen in the end
    counts, bins = np.histogram(data, num_bins)
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


def getZtheta(list_of_ps_log):
    # list_of_ps_log is a list in the form [[s1, ps_log1], [s2, ps_log2], ..]
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
            logging.warning("not enough overlap, insert an intermediate theta")

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

    for N in steps:
        list_of_ps_log = []
        for T in thetas:
            name = param.basename.format(steps=N,
                                         theta=T,
                                         **param.parameters
                                         )
            data = getDistribution("{}/{}.dat".format(d, name),
                                   "{}/dist_{}.dat".format(out, name),
                                   "{}/hist_{}.dat".format(out, name),
                                   T, N)
            list_of_ps_log.append(data)
            z = getZtheta(list_of_ps_log)
            #~ outfiles.append('"{}/dist_{}.dat"'.format(out, name))
            outfiles.append('"{}/stiched_{}.dat"'.format(out, name))

        zc = [0]
        zce = [0]
        for i in z[1:]:
            zc.append(zc[-1] + i[0])
            zce.append(zce[-1] + i[1])

        whole_distribution = []
        for n, T in enumerate(thetas):
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


if __name__ == "__main__":
    run()
