#!/usr/bin/env python3

import sys
import logging
from subprocess import call

import parameters as param
from config import SimulationInstance


logging.basicConfig(level=logging.INFO,
                format='%(asctime)s -- %(levelname)s :: %(message)s',
                datefmt='%d.%m.%YT%H:%M:%S')
logging.info("started")


def run(*args, **kwargs):
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
    iterations = param.parameters["iterations"]

    outfiles = []

    column = param.parameters["observable"]
    if column >= 3:
        column = 1

    for N in steps:
        logging.info("N = {}".format(N))

        outbase = param.basename.format(steps=N, **param.parameters)

        try:
            theta_for_N = thetas[N]
        except KeyError:
            theta_for_N = thetas[0]

        # find names of needed files
        nameDict = {}
        if sampling == 1:
            for T in theta_for_N:
                i = SimulationInstance(steps=N, theta=T, **param.parameters)
                nameDict.update({T: i.basename})
        elif sampling == 4 or sampling == 5:
            i = SimulationInstance(steps=N, theta=theta_for_N, **param.parameters)
            for T, bn in zip(i.T, i.basename):
                nameDict.update({T: bn})
        else:
            logging.error("unkown sampling method")

        # read all the files -- in parallel (also adjust for autocorrelation)
        cmd = ["./glue++"]
        cmd += ["-f", "--bootstrap"]
        cmd += ["-t", "10"]
        cmd += ["-s", "{:.0f}".format(0.1*iterations)]
        cmd += ["-c", "{}".format(column)]
        cmd += ["-o", "{}/whole_{}.dat".format(out, outbase)]
        cmd += ["-L", "{}/whole_{}.log".format(out, outbase)]
        for T in theta_for_N:
            cmd += ["-i", "{}".format("{}/{}.dat.gz".format(d, nameDict[T]))]
            if T == float("inf"):
                T = 1e34
            cmd += ["-T", "{}".format(T)]

        call(cmd)

if __name__ == "__main__":
    logging.info("started Metropolis evaluation")
    run()
