#!/usr/bin/env python3

from math import exp, log, ceil

import numpy as np
from scipy.interpolate import interp1d

import parameters as param
from config import bootstrap, bootstrap_histogram, histogram_simple_error


def getAutocorrTime(data):
    # TODO: calculate the autocorrelation time
    autocorr = np.correlate(data-np.mean(data), data-np.mean(data), mode='full')[len(data)-1:]
    x0 = autocorr[0]

    # integrate only up to the first negative values
    #~ m = -1
    #~ for n, i in enumerate(autocorr):
        #~ if i < 0:
            #~ m = n
            #~ break

    tau = sum(autocorr)/x0
    print("autocorrelation time:", tau)
    return tau

def getDistribution(infile, outfile, theta, steps):
    try:
        a = np.loadtxt(infile+".gz")
    except FileNotFoundError:
        try:
            a = np.loadtxt(infile)
        except FileNotFoundError:
            print("cannot find " + self.filename)
            return

    col = param.parameters["observable"]

    data = a.transpose()[col]
    t_corr = getAutocorrTime(data)
    # do only keep statistically independent samples to not underestimate the error
    data = data[::ceil(2*t_corr)]

    counts, bins = np.histogram(data, 50, density=True)
    centers = (bins[1:] + bins[:-1])/2
    centers_err = [(centers[i] - bins[i]) / 2 for i in range(len(centers))]

    bs_mean, bs_err = bootstrap_histogram(data, bins=bins, density=True)
    #bs_err = histogram_simple_error(counts)

    data = []
    for s, s_err, pts, pts_err in zip(centers, centers_err, bs_mean, bs_err):
        #~ print(i, exp(i/theta) * j)
        try:
            ps_log = s/theta + log(pts)
            # gaussian erroro propagation
            ps_log_err = pts_err/pts  + s_err/theta
            if pts_err/pts > s_err/theta:
                # ignore too noisy bins
                #~ raise ValueError
                pass
        except ValueError:
            pass
        else:
            data.append((s, s_err, ps_log, ps_log_err))

    print(outfile)
    with open(outfile, "w") as f:
        f.write("# S S_err P(S) P(S)_err\n")
        for d in data:
            f.write("{} {} {} {}\n".format(*d))

    return data


def getZtheta(list_of_ps_log):
    # list_of_ps_log is a list in the form [[s1, ps_log1], [s2, ps_log2], ..]
    Ztheta_mean = [(0, 0)]
    for i in range(len(list_of_ps_log)-1):
        l1 = list_of_ps_log[i]
        l2 = list_of_ps_log[i+1]

        # calculate a cubic spline through l1
        x, x_err, y, y_err = zip(*l1)
        spline_1 = interp1d(x, y, kind='cubic')

        # calculate for every s in l2 the difference to l2(s) - spline_l1(s)
        Z = [ps_log - spline_1(s) for s, _, ps_log, _ in l2 if min(x) < s < max(x)]
        Ztheta_mean.append(bootstrap(Z))

    return Ztheta_mean


def stichFile(infile, outfile, z):
    with open(infile, "r") as fin:
        with open(outfile, "w") as fout:
            for line in fin.readlines():
                if line[0] == "#":
                    continue
                nums = list(map(float, line.split()))
                nums[2] -= z
                fout.write("{} {} {} {}\n".format(*nums))


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
            data = getDistribution("{}/{}.dat".format(d, name), "{}/dist_{}.dat".format(out, name), T, N)
            list_of_ps_log.append(data)
            z = getZtheta(list_of_ps_log)
            #~ outfiles.append('"{}/dist_{}.dat"'.format(out, name))
            outfiles.append('"{}/stiched_{}.dat"'.format(out, name))

        zc = [0]
        for i in z[1:]:
            zc.append(zc[-1] + i[0])

        for n, T in enumerate(thetas):
            name = param.basename.format(steps=N,
                                         theta=T,
                                         **param.parameters
                                         )
            stichFile("{}/dist_{}.dat".format(out, name), "{}/stiched_{}.dat".format(out, name), zc[n])

    print("plot with gnuplot")
    print("p " + ", ".join(i + " u 1:3:2:4 w xyerr" for n, i in enumerate(outfiles)))


if __name__ == "__main__":
    run()
