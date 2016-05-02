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
    if not len(xRaw):
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
    def __init__(self, number_of_steps, thetas, iterations, sampling, **kwargs):

        self.Ns = number_of_steps
        self.n = iterations
        self.sampling = sampling
        self.parallel = kwargs["parallel"]
        self.kwargs = kwargs
        if self.parallel and sampling != 2:
            print("sampling method", sampling, "does not use parallelism, set parallel to None")
            raise

        # Wang landau does not have a theta
        if sampling == 2:
            thetas = {0: (0,)}

        self.instances = []
        for N in number_of_steps:
            try:
                theta_for_N = thetas[N]
            except KeyError:
                theta_for_N = thetas[0]
            for T in theta_for_N:
                self.instances.append(SimulationInstance(steps=N, theta=T, iterations=iterations, sampling=sampling, **kwargs))

    def hero(self):
        logging.info("Create .sge Files for Hero")
        if not os.path.exists("HERO"):
            os.makedirs("HERO")

        # in MB
        def getMem(N):
            if N < 1000:
                return 100
            else:
                return 500


        # time per sweep
        def getSec(N):
            if self.sampling == 1:
                t = 0 # time for 1000 sweeps
                if N <= 64:
                    t = 5
                elif N <= 128:
                    t = 15
                elif N <=256:
                    t = 50
                elif N <= 512:
                    t = 250
                elif N <= 1024:
                    t = 1500
                elif N <= 2048:
                    t = 15000
                else:
                    t = 50000

                return t/1000 * 3 # factor 3 to be sure

            if self.sampling == 2:
                # no data yet
                if N <= 30:
                    t = 10
                elif N <= 200:
                    t = 7000
                else:
                    t = 86000*3  # say, 3 days

                return t/self.parallel * 3 # factor 3 to be sure

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
                f.write(template.render(name=name,
                                        count=ctr,
                                        hours=math.ceil(getSec(N)*self.n/3600*2),
                                        mb=getMem(N),
                                        parallel=self.parallel))

    # start the calculation
    def __call__(self):
        logging.info("Executing {} jobs".format(len(self.instances)))
        # run only one Wang landau at a time, but many metropolis
        if self.sampling == 2:
            for i in self.instances:
                i()
        else:
            with Pool() as p:
                p.map(run_instance, self.instances)


class SimulationInstance():
    def __init__(self, steps, typ, seedMC, seedR, iterations,
                       dimension, theta, t_eq, t_corr, directory,
                       rawData, rawConf, observable,
                       method, akl, sampling, parallel, **not_used):

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
        self.m = sampling
        self.parallel = parallel
        self.t_eq = t_eq
        self.t_corr = t_corr

        self.loadFile = None

        if not os.path.exists(self.rawData):
            os.makedirs(self.rawData)
        if not os.path.exists(self.d):
            os.makedirs(self.d)
        if not os.path.exists(self.rawData):
            os.makedirs(self.rawData)
        if self.rawConf and not os.path.exists(self.rawConf):
            os.makedirs(self.rawConf)

        self.basename = para.basename.format(typ=self.t, steps=self.N, seedMC=self.x, seedR=self.y, theta=self.T, iterations=self.n, observable=self.w, sampling=self.m, dimension=self.D)
        self.filename = "{}/{}.dat".format(self.rawData, self.basename)
        if self.rawConf:
            self.confname = "{}/{}.dat".format(self.rawConf, self.basename)

    def __str__(self):
        return "RW:\n\tN = {}\n\tt={}".format(self.N, self.t)

    def __repr__(self):
        return "RW:N={}.t={}.T={}".format(self.N, self.t, self.T)

    def get_cmd(self):
        try:
            it = int(self.n * self.t_corr[self.N][self.T])
        except KeyError:
            pass
        else:
            it = self.n

        opts = ["./randomWalk",
                "-N {0}".format(self.N),
                "-x {0}".format(self.x),
                "-y {0}".format(self.y),
                "-T {0:.5f}".format(self.T),
                "-n {0}".format(it),
                "-c {:d}".format(self.method),
                "-d {:d}".format(self.D),
                "-t {0}".format(self.t),
                "-w {0}".format(self.w),
                "-q",
                "-o {0}".format(self.filename),
                "-m {0}".format(self.m),
               ]

        if self.rawConf:
            opts.append("-O {0}".format(self.confname))

        try:
            opts.append("--t_eq {0:.0f}".format(self.t_eq[self.N][self.T]))
        except KeyError:
            pass

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
