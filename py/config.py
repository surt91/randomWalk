import os
from math import sqrt
from subprocess import call
from multiprocessing import Pool
import warnings
import math
import logging

import numpy as np
import jinja2

import parameters as para

def run_instance(i):
    i()

def bootstrap(xRaw, n_resample=100, f=np.mean):
    """Bootstrap resampling, returns mean and stderror"""
    if not xRaw:
        return float("NaN"), float("NaN")
    bootstrapSample = [f(np.random.choice(xRaw, len(xRaw), replace=True)) for i in range(n_resample)]
    return np.mean(bootstrapSample), np.std(bootstrapSample)

def bootstrap_histogram(xRaw, bins, n_resample=100, density=False):
    """Bootstrap resampling, returns mean and stderror"""
    if not len(xRaw):
        return float("NaN"), float("NaN")
    allCounts = np.zeros((n_resample, len(bins)-1), dtype=np.float)
    for i in range(n_resample):
        allCounts[i], _ = np.histogram(np.random.choice(xRaw, len(xRaw), replace=True), bins=bins, density=density)
    return np.mean(allCounts, 0), np.std(allCounts, 0)

def histogram_simple_error(counts):
    return np.sqrt(counts)

def binder(a):
    if(np.mean(a) == 0):
        return 0
    return (3-np.mean(a**4)/np.mean(a**2)**2)/2

def file_not_empty(fpath):
    return True if os.path.isfile(fpath) and os.path.getsize(fpath) > 0 else False


class Simulation():
    def __init__(self, number_of_steps, typ, seedMC, seedR, iterations,
                       dimension, thetas, directory, rawData, rawConf,
                       observable, method, akl):

        self.Ns = number_of_steps
        self.n = iterations

        self.instances = []
        for N in number_of_steps:
            for T in thetas:
                self.instances.append(SimulationInstance(steps=N, typ=typ, seedMC=seedMC, seedR=seedR, iterations=iterations,
                       dimension=dimension, theta=T, directory=directory, rawData=rawData, rawConf=rawConf,
                       observable=observable, method=method, akl=akl))

    def hero(self):
        logging.info("Create .sge Files for Hero")
        if not os.path.exists("HERO"):
            os.makedirs("HERO")

        # in MB
        def getMem(N):
            return 1000

        # time per sweep
        def getSec(N):
            return 1

        self.env = jinja2.Environment(trim_blocks=True,
                                      lstrip_blocks=True,
                                      loader=jinja2.FileSystemLoader("templates"))
        template = self.env.get_template("jobarray.sge")

        # create .sge files for Hero
        for N in self.Ns:
            ctr = 0
            name = "RW{:d}".format(N)
            with open(os.path.join("HERO", name+".lst"), "w") as f:
                for i in self.instances:
                    if i.N == N:
                        f.write(" ".join(i.get_cmd()) + "\n")
                        ctr += 1
            with open(os.path.join("HERO", name+".sge"), "w") as f:
                f.write(template.render(name=name, count=ctr, hours=math.ceil(getSec(N)*self.n/3600*2), mb=getMem(N)))

    # start the calculation
    def __call__(self):
        logging.info("Executing {} jobs".format(len(self.instances)))
        with Pool() as p:
            p.map(run_instance, self.instances)


class SimulationInstance():
    def __init__(self, steps, typ, seedMC, seedR, iterations,
                       dimension, theta, directory, rawData, rawConf,
                       observable, method, akl):

        self.N = steps
        self.n = iterations
        self.t = typ
        self.T = theta
        self.x = seedMC
        self.y = seedR
        self.D = dimension
        self.rawData = rawData
        self.rawConf = rawConf
        self.d = directory
        self.method = method
        self.akl = akl
        self.w = observable

        self.loadFile = None

        if not os.path.exists(self.rawData):
            os.makedirs(self.rawData)
        if not os.path.exists(self.d):
            os.makedirs(self.d)
        if not os.path.exists(self.rawData):
            os.makedirs(self.rawData)
        if self.rawConf and not os.path.exists(self.rawConf):
            os.makedirs(self.rawConf)

        self.basename = para.basename.format(typ=self.t, steps=self.N, seedMC=self.x, seedR=self.y, theta=self.T, iterations=self.n, observable=self.w)
        self.filename = "{}/{}.dat".format(self.rawData, self.basename)
        if self.rawConf:
            self.confname = "{}/{}.dat".format(self.rawConf, self.basename)

    def __str__(self):
        return "RW:\n\tN = {}\n\tt={}".format(self.N, self.t)

    def __repr__(self):
        return "RW:N={}.t={}.T={}".format(self.N, self.t, self.T)

    def get_cmd(self):
        opts = ["./randomWalk",
                "-N {0}".format(self.N),
                "-x {0}".format(self.x),
                "-y {0}".format(self.y),
                "-T {0:.5f}".format(self.T),
                "-n {0}".format(self.n),
                "-c {:d}".format(self.method),
                "-d {:d}".format(self.D),
                "-t {0}".format(self.t),
                "-w {0}".format(self.w),
                "-q",
                "-o {0}".format(self.filename),
               ]

        if self.rawConf:
            opts.append("-O {0}".format(self.confname))

        if self.akl:
            opts.append("-a")

        #~ if self.loadFile:
            #~ opts.append("-f {0}".format(self.loadFile))

        return opts

    def set_load_file(self, loadfile):
        self.loadFile = loadfile
        self.t = 13

    def __call__(self):
        if not os.path.exists(self.filename+".gz"):
            if 0 != call(self.get_cmd(), stdout=None, stderr=None):
                logging.error("Error in command '%s'" % (" ".join(self.get_cmd())))
            print(".", flush=True, end="")
        else:
            print("-", flush=True, end="")
        print("")
