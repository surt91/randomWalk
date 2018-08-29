import os
import subprocess
from subprocess import call
from multiprocessing import Pool
import math
import logging
import operator
import copy
import pickle

import numpy as np
import jinja2

import parameters as para
from timing import time


def run_instance(i):
    i.quiet = True
    i()


def bootstrap(xRaw, f=np.mean, n_resample=100):
    """Bootstrap resampling, returns mean and stderror"""
    if not len(xRaw):
        return float("NaN"), float("NaN")
    bootstrapSample = [f(np.random.choice(xRaw, len(xRaw), replace=True)) for i in range(n_resample)]
    return np.mean(bootstrapSample), np.std(bootstrapSample)


def bs_wrapper(tup):
    i, xRaw, f, kwargs = tup
    np.random.seed(i)
    newDict = {k: np.random.choice(v, len(v), replace=True) for k, v in xRaw.items()}
    return f(newDict, **kwargs)


def bootstrap_dict(xRaw, N, f=np.histogram, n_resample=100, parallelness=1, **kwargs):
    """Bootstrap resampling, reduction function takes a list and returns
    a list of len N. Returns a list of means and a list of errors.

    :param xRaw:    vector of raw input data
    :param N:       length of list returned by f
    :param f:       reduction function, takes a list, returns a list
    :param kwargs:  keyword arguments for f
    """
    if not len(xRaw):
        return float("NaN"), float("NaN")

    # do the bootstrapping in parallel, if parallelness is given
    with Pool(parallelness) as p:
        tmp = p.map(bs_wrapper, [(i, xRaw, f, kwargs) for i in range(n_resample)])

    # copy results to np.array
    allCounts = np.zeros((n_resample, N), dtype=np.float)
    for n, i in enumerate(tmp):
        allCounts[n] = i

    return np.mean(allCounts, 0), np.std(allCounts, 0)


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
    return os.path.isfile(fpath) and os.path.getsize(fpath) > 0


def WL_check_overlap(N, instances_for_N, centers):
    logging.info("checking overlap for N = {}".format(N))
    # test if there is enough overlap
    for i in range(len(centers)-1):
        Z = sum(1 for x in centers[i+1] if min(centers[i]) < x < max(centers[i]))
        # not enough overlap
        if Z < 5:
            logging.warning("not enough overlap ({}: {} -- {})".format(Z, max(centers[i]), min(centers[i+1])))


def WL_check_bounds(N, instances_for_N, centers):
    logging.info("checking bounds for  N = {}".format(N))
    try:
        # load cached bounds
        with open("cache_bounds_{}.p".format(N), "rb") as f:
            bounds = pickle.load(f)
    except:
        # if we can not load them, recalculate them
        # restart 4 times
        # we do not want to get a too good bound, otherwise the
        # simulation will not finish.
        get_bounds_tmp = operator.methodcaller('get_WL_bounds')
        tests = [copy.deepcopy(instances_for_N[0]) for _ in range(4)]
        # update seeds
        for n, i in enumerate(tests):
            tests[n].x = n
            tests[n].y = n
        with Pool() as pool:
            bounds = pool.map(get_bounds_tmp, tests)
        bounds = min(i[0] for i in bounds), max(i[1] for i in bounds)
        with open("cache_bounds_{}.p".format(N), "wb") as f:
            pickle.dump(bounds, f)

    logging.info("estimated bounds: [{}, {}]".format(*bounds))
    # test if the ranges are inside the bounds
    if bounds[0] > min(min(c) for c in centers):
        logging.warning("there are bins below the lower bound ({} > {})".format(bounds[0], min(min(c) for c in centers)))
        logging.warning("This could result in an infinite loop")
    if bounds[1] < max(max(c) for c in centers):
        logging.warning("there are bins above the upper bound ({} < {})".format(bounds[1], max(max(c) for c in centers)))
        logging.warning("This could result in an infinite loop")


class Simulation():
    def __init__(self, number_of_steps, thetas, iterations, passageTimeStart=(-1,), **kwargs):

        self.Ns = number_of_steps
        self.n = iterations
        self.sampling = kwargs["sampling"]
        self.num_batches = kwargs["batches"]
        self.parallel = kwargs["parallel"]
        if self.parallel is None:
            self.parallel = 1
        p = self.parallel
        energies = kwargs["energies"]
        self.kwargs = kwargs
        if self.parallel > 1 and (self.sampling != 2 and self.sampling != 3 and self.sampling != 4):
            print("sampling method", self.sampling, "does not use parallelism, set parallel to None")
            raise

        if kwargs["observable"] != 3:
            passageTimeStart = (-1,)

        if self.sampling == 0:
            x = kwargs["seedMC"]
            y = kwargs["seedR"]
            del kwargs["seedMC"]
            del kwargs["seedR"]

        self.instances = []
        for N in number_of_steps:
            for t1 in passageTimeStart:
                if self.sampling == 0:
                    for i in range(self.num_batches):
                        x += 1
                        y += 1
                        batch_iterations = iterations / self.num_batches
                        if i == self.num_batches - 1:
                            batch_iterations += iterations % self.num_batches
                        self.instances.append(SimulationInstance(steps=N, iterations=batch_iterations, passageTimeStart=t1, batch_id=i, seedMC=x, seedR=y, **kwargs))
                if self.sampling == 1:
                    for T in thetas[N]:
                        self.instances.append(SimulationInstance(steps=N, theta=T, iterations=iterations, passageTimeStart=t1, **kwargs))
                if self.sampling == 4 or self.sampling == 5:
                    self.instances.append(SimulationInstance(steps=N, theta=thetas[N], iterations=iterations, passageTimeStart=t1, **kwargs))
                if self.sampling == 2 or self.sampling == 3:
                    num = len(energies[N])-1
                    instances_for_N = []
                    for i in range(num):
                        instances_for_N.append(SimulationInstance(steps=N, energy=list(energies[N][i:i+p+1]), iterations=iterations, passageTimeStart=t1, first=not i, last=(i==num-1), **kwargs))
                    self.instances += instances_for_N

                    # call the executable to ask for the centers in parallel
                    with Pool() as pool:
                        get_centers_tmp = operator.methodcaller('get_WL_centers')
                        centers = pool.map(get_centers_tmp, instances_for_N)

                    WL_check_overlap(N, instances_for_N, centers)
                    WL_check_bounds(N, instances_for_N, centers)

    def ihero(self):
        return self.hero(True)

    def hero(self, incremental=False):
        logging.info("Create .sge and .slurm Files for HPC")
        if not os.path.exists("HPC"):
            os.makedirs("HPC")

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
                t = 0  # time for 1000 sweeps
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
                    elif N <= 256:
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

                return t/self.parallel * 3  # factor 3 to be sure

            return 86000*3  # 3 days default

        self.env = jinja2.Environment(trim_blocks=True,
                                      lstrip_blocks=True,
                                      loader=jinja2.FileSystemLoader("templates"))

        # create .sge files for Hero
        for N in self.Ns:
            ctr = 0
            name = "RW{:d}".format(N)
            with open(os.path.join("HPC", name+".lst"), "w") as f:
                for i in self.instances:
                    if i.N == N:
                        # for wang landau sampling, split the iterations into
                        # own processes
                        if self.sampling == 2 or self.sampling == 3:
                            for k in range(1, self.n+1):
                                sim = copy.copy(i)
                                sim.n = 1
                                sim.filename = i.filename + ".{}".format(k)
                                sim.logname = i.logname + ".{}".format(k)
                                # we need to change the seeds, otherwise
                                # we will get n identical results
                                sim.x += k
                                sim.y += k
                                if not incremental or not os.path.exists(sim.filename+".gz"):
                                    f.write(" ".join(sim.get_cmd()) + "\n")
                                    ctr += 1
                        else:
                            if not incremental or not os.path.exists(i.filename+".gz"):
                                f.write(" ".join(i.get_cmd()) + "\n")
                                ctr += 1
            if self.sampling == 5:
                with open(os.path.join("HPC", name+".slurm"), "w") as f:
                    template = self.env.get_template("jobarray_mpi.slurm")
                    f.write(template.render(name=name,
                                            numTemperatures=len(self.temperatures),
                                            hours=math.ceil(getSec(N)*self.n/3600*2),
                                            mb=getMem(N)))
            else:
                with open(os.path.join("HPC", name+".slurm"), "w") as f:
                    template = self.env.get_template("jobarray.slurm")
                    f.write(template.render(name=name,
                                            count=ctr,
                                            hours=math.ceil(getSec(N)*self.n/3600*2),
                                            mb=getMem(N),
                                            parallel=self.parallel))
                with open(os.path.join("HPC", name+".bsub"), "w") as f:
                    template = self.env.get_template("jobarray.bsub")
                    f.write(template.render(name=name,
                                            count=ctr,
                                            hours=math.ceil(getSec(N)*self.n/3600*2),
                                            mb=getMem(N),
                                            parallel=self.parallel))

    # start the calculation
    def __call__(self):
        logging.info("Executing {} jobs".format(len(self.instances)))

        # 0 means: use all cores
        if self.parallel > 1 or self.parallel == 0 or self.sampling == 5:
            # program is multithreaded or multiprocess, start only one
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

        :param steps:      []      (int)         number of steps
        :param typ:        []      (int)         type of walk
        :param seedMC:     []      (int)         seed for the MC simulation
        :param seedR:      []      (int)         seed for the start realization
        :param iterations: []      (int)         number of sweeps to perform
        :param sweep:      [1,4]   (int)         number of trial move per sweep
        :param dimension:  []      (int)         spacial dimension
        :param t_eq:       [1,4]   (int)         sweeps for equilibration (-1 for auto config)
        :param t_corr:     [1,4]   (int)         sweeps to decorrelate
        :param directory:  []      (str)         folder to store the processed data
        :param rawData:    []      (str)         folder where the rawData is located
        :param rawConf:    []      (str)         folder where the rawConf is located
        :param observable: []      (int)         which observable to measure (volume, or surface)
        :param method:     []      (int)         convex hull algorithm to use
        :param akl:        []      (bool)        use akl heuristic
        :param sampling:   []      (int)         sampling method to use
        :param parallel:   [2,3,4] (int)         how many threads to use for the simulation (OpenMP)
        :param energy:     [2,3]   (list[float]) borders of the energy ranges
        :param nbins:      [2,3]   (int)         how many bins in each energy range
        :param overlap:    [2,3]   (int)         how many bins to extend beyond the energy range
        :param lnf:        [2,3]   (float)       ln of the refinement factor
        :param flatness:   [2,3]   (float)       flatness criterion
        :param overlap_direction: [2,3] (str)    in which direction to extend beyond the range ("left"/"right")
        :param t_eq_max:   [1]     (int)         how long to search for quilibration before aborting
        :param theta:      [1]     (float)       temperature to use for the simulation
        :param theta:      [4]     (list[float]) temperatures to use for the simulation
        :param first:      [2,3]   (bool)        is this the first one? (to not extend range to the left)
        :param last:       [2,3]   (bool)        is this the last one? (to not extend range to the right)
        :param number_of_walkers: [] (int)       number of independed walkers
        :param passageTimeStart: [] (int)        time point reference after which to look for sign changes
    """
    def __init__(self, steps, typ, seedMC, seedR, iterations,
                       dimension, t_eq, t_corr, directory,
                       rawData, rawConf, observable,
                       method, akl, sampling, parallel, nbins, overlap,
                       lnf, flatness, overlap_direction="left",
                       t_eq_max=None, theta=None, energy=None,
                       first=False, last=False, sweep=None,
                       number_of_walkers=None, passageTimeStart=-1,
                       batch_id=0, beta=1.0, reset=0.0, **not_used):

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
        self.passageTimeStart = passageTimeStart
        self.beta = beta
        self.reset = reset
        self.quiet = False

        self.first = first
        self.last = last

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
        elif sampling == 4 or sampling == 5:
            pass  # we only have one process and do not need to change the seeds
        elif sampling == 2 or sampling == 3:
            self.x += int(self.energy[0])
            self.y += int(self.energy[0])

        # do the modulo with a big prime, to avoid overflows and repitions
        # modulo not well defined for negative values -> abs
        self.x = abs(self.x) % 1700000333
        self.y = abs(self.y) % 1700000339

        if sampling == 0:
            self.basename = para.basesimple.format(typ=self.t, steps=self.N, seedR=self.y, batch=batch_id, iterations=self.n, observable=self.w, sampling=self.m, dimension=self.D, passageTimeStart=self.passageTimeStart, beta=self.beta, reset=self.reset)
        elif sampling == 1:
            self.basename = para.basetheta.format(typ=self.t, steps=self.N, seedMC=self.x, seedR=self.y, theta=self.T, iterations=self.n, observable=self.w, sampling=self.m, dimension=self.D, passageTimeStart=self.passageTimeStart, beta=self.beta, reset=self.reset)
        elif sampling == 4 or sampling == 5:
            self.basename = []
            for T in self.T:
                self.basename.append(para.basetheta.format(typ=self.t, steps=self.N, seedMC=self.x, seedR=self.y, theta=T, iterations=self.n, observable=self.w, sampling=self.m, dimension=self.D, passageTimeStart=self.passageTimeStart, beta=self.beta, reset=self.reset))
        elif sampling == 2 or sampling == 3:
            self.basename = para.basee.format(typ=self.t, steps=self.N, seedMC=self.x, seedR=self.y, estart=self.energy[0], eend=self.energy[-1], iterations=self.n, observable=self.w, sampling=self.m, dimension=self.D, passageTimeStart=self.passageTimeStart, beta=self.beta, reset=self.reset)

        if sampling == 4 or sampling == 5:
            self.filename = []
            self.logname = para.basename.format(typ=self.t, steps=self.N, seedMC=self.x, seedR=self.y, iterations=self.n, observable=self.w, sampling=self.m, dimension=self.D, passageTimeStart=self.passageTimeStart, beta=self.beta, reset=self.reset) + ".log"
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
            for l in out.stdout.split("\n"):
                if l.startswith("Warning") or l.startswith("Error"):
                    print(" ".join(cmd))
                    print(l)
            outlines = [l for l in out.stdout.split("\n") if not l.startswith("#") and not l.startswith("Info") and not l.startswith("Warning") and l]
            # centers = [list(map(float, i.split())) for i in outlines]
            # print(out.stdout)
            # print(outlines[0].split())
            centers = list(map(float, outlines[0].split()))
        else:
            raise

        return centers

    def get_WL_bounds(self):
        cmd = self.get_cmd(logging=False)
        cmd += ["--onlyBounds"]
        out = subprocess.run(cmd, stdout=subprocess.PIPE, universal_newlines=True)

        lower, upper = None, None
        for l in out.stdout.split("\n"):
            if l.startswith("min: "):
                lower = float(l.split()[1])
            if l.startswith("max: "):
                upper = float(l.split()[1])

        if lower is None or upper is None:
            raise

        return lower, upper

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
                "--beta {}".format(self.beta),
                "--reset {}".format(self.reset),
               ]

        if self.m == 4 or self.m == 5:
            for f in self.filename:
                opts.append("-o {}".format(f))
        else:
            opts.append("-o {}".format(self.filename))

        if self.m == 5:
            opts = ["mpirun", "-n", "{}".format(len(self.T))] + opts

        if logging:
            opts.append("-L {}".format(self.logname))

        if self.number_of_walkers:
            opts.append("-M {}".format(self.number_of_walkers))

        if self.quiet:
            opts.append("-q")

        if self.parallel:
            opts.append("-P {}".format(self.parallel))

        if self.rawConf:
            opts.append("-O {0}".format(self.confname))

        try:
            opts.append("--t_eq {0:.0f}".format(self.t_eq[self.N][self.T]))
        except KeyError:
            pass
        except TypeError:
            opts.append("--t_eq {0:.0f}".format(self.t_eq[self.N]))
        if self.t_eq_max:
            opts.append("--t_eq_max {0:.0f}".format(self.t_eq_max))

        if self.akl:
            opts.append("-a")

        try:
            for i in self.passageTimeStart:
                if self.N < i:
                    i = 1  # some dummy
                opts.append("-z {:.0f}".format(i))
        except ValueError:
            pass
        except TypeError:
            pass

        if self.m == 1 or self.m == 4 or self.m == 5:
            if self.sweep[self.N]:
                opts.append("-k {:.0f}".format(self.sweep[self.N]))

        if self.m == 1:
            if self.T != float("inf"):
                opts.append("-T {0:.5f}".format(self.T))
            else:
                opts.append("--simplesampling")
        elif self.m == 4 or self.m == 5:
            for T in self.T:
                if T != float("inf"):
                    opts.append("-T {0:.5f}".format(T))
                else:
                    opts.append("-T {0:.5f}".format(1.417e32))  # Planck temperature ;)

        elif self.m == 2 or self.m == 3:
            for e in self.energy:
                opts.append("-e {}".format(e))

            if self.overlap_direction == "right":
                if self.last:
                    opts.append("-B {}".format(self.nbins-self.overlap))
                else:
                    opts.append("-B {}".format(self.nbins))
            else:
                if self.first:
                    opts.append("-B {}".format(self.nbins-self.overlap))
                else:
                    opts.append("-B {}".format(self.nbins))

            opts.append("-E {}".format(self.overlap))
            opts.append("--lnf {}".format(self.lnf))
            opts.append("--flatness {}".format(self.flatness))

        # if self.loadFile:
        #     opts.append("-f {0}".format(self.loadFile))

        return opts

    def set_load_file(self, loadfile):
        self.loadFile = loadfile
        self.t = 13

    def __call__(self):
        unfinished = True
        if self.m == 4 or self.m == 5:
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
