import os
from math import sqrt
import subprocess
from subprocess import call
from multiprocessing import Pool
import warnings
import math
import logging
import operator

import numpy as np
import jinja2

import parameters as para
from timing import time

def run_instance(i):
    i.quiet = True
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
    def __init__(self, number_of_steps, thetas, iterations, **kwargs):

        self.Ns = number_of_steps
        self.n = iterations
        self.sampling = kwargs["sampling"]
        self.parallel = kwargs["parallel"]
        if self.parallel is None:
            self.parallel = 1
        p = self.parallel
        energies = kwargs["energies"]
        self.kwargs = kwargs
        if self.parallel and (self.sampling != 2 and self.sampling != 3 and self.sampling != 4):
            print("sampling method", self.sampling, "does not use parallelism, set parallel to None")
            raise

        self.instances = []
        for N in number_of_steps:
            if self.sampling == 1:
                for T in thetas[N]:
                    self.instances.append(SimulationInstance(steps=N, theta=T, iterations=iterations, **kwargs))
            if self.sampling == 4:
                self.instances.append(SimulationInstance(steps=N, theta=thetas[N], iterations=iterations, **kwargs))
            if self.sampling == 2 or self.sampling == 3:
                num = len(energies[N])-1
                instances_for_N = []
                for i in range(num):
                    instances_for_N.append(SimulationInstance(steps=N, energy=list(energies[N][i:i+p+1]), iterations=iterations, first=not i, last=(i==num-1), **kwargs))
                self.instances += instances_for_N

                logging.info("checking overlap for N = {}".format(N))
                # call the executable to ask for the centers in parallel
                with Pool() as pool:
                    get_centers_tmp = operator.methodcaller('get_WL_centers')
                    centers = pool.map(get_centers_tmp, instances_for_N)
                # test if there is enough overlap
                for i in range(len(centers)-1):
                    Z = sum(1 for x in centers[i+1] if min(centers[i]) < x < max(centers[i]))
                    # not enough overlap
                    if Z < 3:
                        logging.warning("not enough overlap ({}: {} -- {})".format(Z, max(centers[i]), min(centers[i+1])))

    def ihero(self):
        return self.hero(True)

    def hero(self, incremental=False):
        logging.info("Create .sge Files for Hero")
        if not os.path.exists("HERO"):
            os.makedirs("HERO")

        # on Hero None means single threaded
        if self.parallel is None:
            self.parallel = 1

        # in MB
        def getMem(N):
            if N < 1000:
                m = 100
            else:
                m = 300

            return m*self.parallel

        # time per sweep
        def getSec(N):
            if self.sampling == 1 or self.sampling == 4:
                t = 0 # time for 1000 sweeps
                max_t_corr = 1
                try:
                    # correct according to maximum t_corr
                    max_t_corr = max(para.t_corr[N].values())
                except KeyError:
                    pass
                except ValueError:
                    pass
                try:
                    t = time[self.kwargs["typ"]][self.kwargs["dimension"]][N]
                except:
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
                return t/1000 * max_t_corr

            if self.sampling == 2 or self.sampling == 3:
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
                        if not incremental or not os.path.exists(i.filename+".gz"):
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

        # 0 means: use all cores
        if self.parallel > 1 or self.parallel == 0:
            # program is multithreaded, start only one
            for i in self.instances:
                run_instance(i)
        else:
            # program is single threaded, we do the parallelness
            with Pool() as p:
                # chunksize of 1 for such for optimal parallelness
                p.map(run_instance, self.instances, 1)


class SimulationInstance():
    """One Simulation instance

        In [] are the sampling methods where the option is applicable. empty, if all.
        In () is the data type (e.g. (list[int]))

        :param:steps:      []      (int)         number of steps
        :param:typ:        []      (int)         type of walk
        :param:seedMC:     []      (int)         seed for the MC simulation
        :param:seedR:      []      (int)         seed for the start realization
        :param:iterations: []      (int)         number of sweeps to perform
        :param:sweep:      [1,4]   (int)         number of trial move per sweep
        :param:dimension:  []      (int)         spacial dimension
        :param:t_eq:       [1,4]   (int)         sweeps for equilibration (-1 for auto config)
        :param:t_corr:     [1,4]   (int)         sweeps to decorrelate
        :param:directory:  []      (str)         folder to store the processed data
        :param:rawData:    []      (str)         folder where the rawData is located
        :param:rawConf:    []      (str)         folder where the rawConf is located
        :param:observable: []      (int)         which observable to measure (volume, or surface)
        :param:method:     []      (int)         convex hull algorithm to use
        :param:akl:        []      (bool)        use akl heuristic
        :param:sampling:   []      (int)         sampling method to use
        :param:parallel:   [2,3,4] (int)         how many threads to use for the simulation (OpenMP)
        :param:energy:     [2,3]   (list[float]) borders of the energy ranges
        :param:nbins:      [2,3]   (int)         how many bins in each energy range
        :param:overlap:    [2,3]   (int)         how many bins to extend beyond the energy range
        :param:lnf:        [2,3]   (float)       ln of the refinement factor
        :param:flatness:   [2,3]   (float)       flatness criterion
        :param:overlap_direction: [2,3] (str)    in which direction to extend beyond the range ("left"/"right")
        :param:t_eq_max:   [1]     (int)         how long to search for quilibration before aborting
        :param:theta:      [1]     (float)       temperature to use for the simulation
        :param:theta:      [4]     (list[float]) temperatures to use for the simulation
        :param:first:      [2,3]   (bool)        is this the first one? (to not extend range to the left)
        :param:last:       [2,3]   (bool)        is this the last one? (to not extend range to the right)
        :param:number_of_walkers: [] (int)       number of independed walkers
    """
    def __init__(self, steps, typ, seedMC, seedR, iterations,
                       dimension, t_eq, t_corr, directory,
                       rawData, rawConf, observable,
                       method, akl, sampling, parallel, nbins, overlap,
                       lnf, flatness, overlap_direction="left",
                       t_eq_max=None, theta=None, energy=None,
                       first=False, last=False, sweep=None,
                       number_of_walkers=None, **not_used):

        self.N = steps
        self.number_of_walkers = number_of_walkers
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
        self.t_eq_max = t_eq_max
        self.t_corr = t_corr
        self.energy = energy
        self.nbins = nbins
        self.overlap = overlap
        self.lnf = lnf
        self.flatness = flatness
        self.overlap_direction = overlap_direction
        self.sweep = sweep
        self.quiet = False

        self.loadFile = None

        if not os.path.exists(self.rawData):
            os.makedirs(self.rawData)
        if not os.path.exists(self.d):
            os.makedirs(self.d)
        if not os.path.exists(self.rawData):
            os.makedirs(self.rawData)
        if self.rawConf and not os.path.exists(self.rawConf):
            os.makedirs(self.rawConf)

        if sampling == 2 or sampling == 3:
            old_nbins = nbins
            self.nbins += overlap
            if self.overlap_direction == "right":
                # last one should not overlap to the right
                if not last:
                    # overlap to the right
                    energy[1] += (energy[1] - energy[0]) / old_nbins * overlap
            else:
                # first one should not overlap to the left
                if not first:
                    # overlap to the right
                    energy[0] -= (energy[1] - energy[0]) / old_nbins * overlap

        # change the seeds for every job in the array
        if sampling == 1:
            if self.T == float("inf"):
                t = 0
            else:
                t = self.T
            self.x += int(1e5*t)
            self.y += int(1e5*t)
        elif sampling == 4:
            pass # we only have one process and do not need to change the seeds
        elif sampling == 2 or sampling == 3:
            self.x += int(self.energy[0])
            self.y += int(self.energy[0])

        # do the modulo with a big prime, to avoid overflows and repitions
        # modulo not well defined for negative values -> abs
        self.x = abs(self.x) % 1700000333
        self.y = abs(self.y) % 1700000339

        if sampling == 1:
            self.basename = para.basetheta.format(typ=self.t, steps=self.N, seedMC=self.x, seedR=self.y, theta=self.T, iterations=self.n, observable=self.w, sampling=self.m, dimension=self.D)
        elif sampling == 4:
            self.basename = []
            for T in self.T:
                self.basename.append(para.basetheta.format(typ=self.t, steps=self.N, seedMC=self.x, seedR=self.y, theta=T, iterations=self.n, observable=self.w, sampling=self.m, dimension=self.D))
        elif sampling == 2 or sampling == 3:
            self.basename = para.basee.format(typ=self.t, steps=self.N, seedMC=self.x, seedR=self.y, estart=self.energy[0], eend=self.energy[-1], iterations=self.n, observable=self.w, sampling=self.m, dimension=self.D)

        if sampling == 4:
            self.filename = []
            self.logname = para.basename.format(typ=self.t, steps=self.N, seedMC=self.x, seedR=self.y, iterations=self.n, observable=self.w, sampling=self.m, dimension=self.D) + ".log"
            for bn in self.basename:
                self.filename.append("{}/{}.dat".format(self.rawData, bn))
                if self.rawConf:
                    self.confname = "{}/{}.dat".format(self.rawConf, bn)
        else:
            self.logname = "{}.log".format(self.basename)
            self.filename = "{}/{}.dat".format(self.rawData, self.basename)
            if self.rawConf:
                self.confname = "{}/{}.dat".format(self.rawConf, self.basename)

    def __str__(self):
        return "RW:\n\tN = {}\n\tt={}".format(self.N, self.t)

    def __repr__(self):
        return "RW:N={}.t={}.T={}".format(self.N, self.t, self.T)

    def get_WL_centers(self):
        if self.m == 2 or self.m == 3:
            cmd = self.get_cmd(logging=False)
            cmd += ["--onlyCenters"]
            out = subprocess.run(cmd, stdout=subprocess.PIPE, universal_newlines=True)
            # one line per range
            outlines = [l for l in out.stdout.split("\n") if not l.startswith("#") and not l.startswith("Info") and l]
            #~ centers = [list(map(float, i.split())) for i in outlines]
            #~ print(out.stdout)
            #~ print(outlines[0].split())
            centers = list(map(float, outlines[0].split()))
        else:
            raise

        return centers

    def get_cmd(self, logging=True):
        try:
            it = int(self.n * self.t_corr[self.N][self.T])
        except KeyError:
            it = int(self.n)

        opts = ["./randomWalk",
                "-N {}".format(self.N),
                "-x {}".format(self.x),
                "-y {}".format(self.y),
                "-n {}".format(it),
                "-c {:d}".format(self.method),
                "-d {:d}".format(self.D),
                "-t {}".format(self.t),
                "-w {}".format(self.w),
                "-m {}".format(self.m),
               ]

        if self.m == 4:
            for f in self.filename:
                opts.append("-o {}".format(f))
        else:
            opts.append("-o {}".format(self.filename))

        if logging:
            opts.append("-L {}".format(self.logname))

        if self.number_of_walkers:
            opts.append("-M {}".format(self.number_of_walkers))

        if self.quiet:
            opts.append("-q")

        if self.parallel:
            opts.append("-P {}").format(self.parallel)

        if self.rawConf:
            opts.append("-O {0}".format(self.confname))

        try:
            opts.append("--t_eq {0:.0f}".format(self.t_eq[self.N][self.T]))
        except KeyError:
            pass
        if self.t_eq_max:
            opts.append("--t_eq_max {0:.0f}".format(self.t_eq_max))

        if self.akl:
            opts.append("-a")

        if self.m == 1 or self.m == 4:
            if self.sweep[self.N]:
                opts.append("-k {:.0f}".format(self.sweep[self.N]))

        if self.m == 1:
            if self.T != float("inf"):
                opts.append("-T {0:.5f}".format(self.T))
            else:
                opts.append("--simplesampling")
        elif self.m == 4:
            for T in self.T:
                if T != float("inf"):
                    opts.append("-T {0:.5f}".format(T))
                else:
                    opts.append("-T {0:.5f}".format(1.417e32)) # Planck temperature ;)

        elif self.m == 2 or self.m == 3:
            for e in self.energy:
                opts.append("-e {}".format(e))
            opts.append("-B {}".format(self.nbins))
            opts.append("-E {}".format(self.overlap))
            opts.append("--lnf {}".format(self.lnf))
            opts.append("--flatness {}".format(self.flatness))

        #~ if self.loadFile:
            #~ opts.append("-f {0}".format(self.loadFile))

        return opts

    def set_load_file(self, loadfile):
        self.loadFile = loadfile
        self.t = 13

    def __call__(self):
        unfinished = True
        if self.m == 4:
            # parallel tempering: ensure that all output files exist
            if all(os.path.exists(f+".gz") for f in self.filename):
                unfinished = False
        elif os.path.exists(self.filename+".gz"):
            unfinished = False

        # start simulation, if we have no finished result
        if unfinished:
            if 0 != call(self.get_cmd(), stdout=None, stderr=None):
                logging.error("Error in command '%s'" % (" ".join(self.get_cmd())))
            print(".", flush=True, end="")
        else:
            print("-", flush=True, end="")
