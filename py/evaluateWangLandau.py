#!/usr/bin/env python3

import os
import logging
import gzip
import re
from pathlib import Path
from subprocess import call

import numpy as np
from scipy.interpolate import interp1d
from scipy.integrate import trapz

import parameters as param
from config import bootstrap, bootstrap_histogram, histogram_simple_error, SimulationInstance
from commonEvaluation import getMinMaxTime, getMeanFromDist, getVarFromDist


logging.basicConfig(level=logging.INFO,
                format='%(asctime)s -- %(levelname)s :: %(message)s',
                datefmt='%d.%m.%YT%H:%M:%S')


def run(parallelness=1):
    iterations = param.parameters["iterations"]
    steps = param.parameters["number_of_steps"]
    d = param.parameters["rawData"]
    out = param.parameters["directory"]
    energies = param.parameters["energies"]
    nbins = param.parameters["nbins"]

    for N in steps:
        outname = param.basename.format(steps=N, **param.parameters)
        outbase = "{}/{{}}_{}.dat".format(out, outname)
        if param.parameters["parallel"] is None:
            p = 1
        else:
            p = param.parameters["parallel"]
        num = len(energies[N])-1
        names = ["{}/{}.dat".format(d, SimulationInstance(steps=N, energy=list(energies[N][i:i+p+1]), first=not i, last=(i==num-1), **param.parameters).basename) for i in range(num)]
        names = [n + ".{}.gz".format(i+1) for n in names for i in range(iterations)]

        logging.info("N = {}".format(N))
        # getMinMaxTime((f for f in names), parallelness)

        normed_file = outbase.format("normed")

        cmd = [
            "./glue++",
            "-o {}".format(normed_file),
            "-B {}".format(nbins),
        ]
        cmd += ["-i {}".format(name) for name in names]

        print("".join(cmd))
        call(cmd)


if __name__ == "__main__":
    logging.info("started Wang Landau evaluation")
    run()
