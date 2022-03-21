# randomWalk :game_die:

[![Build Status](https://travis-ci.com/surt91/randomWalk.svg?token=KcmDorpEqtSzJp2wyhgU&branch=master)](https://travis-ci.com/surt91/randomWalk)
[![codecov](https://codecov.io/gh/surt91/randomWalk/branch/master/graph/badge.svg?token=EW5j2uy4vc)](https://codecov.io/gh/surt91/randomWalk)
[![Scc Count Badge](https://sloc.xyz/github/surt91/randomWalk/)](https://github.com/surt91/randomWalk/)

This code is for sampling convex hulls and other properties of a selection of random walks.
Especially it offers advanced sampling methods enabeling sampling of (almost) the whole support based on Metropolis
sampling and Wang-Landau Sampling.

The implemented types of random walks are:

1. Random walk with Gaussian jumps
2. Random walk on a square lattice
3. Self-avoiding random walk
4. Loop-erased random walk
5. Smart kinetic random walk
6. "True" random walk
7. Run and tumble (random walk with exponential jumps, both fixed time and fixed number of tumbles)
8. Levy flight (random walk with power-law jumps)
9. Correlated random walk (random walk with power-law jumps)
10. Resetting random walk (lattice)
11. Resetting random walk (Gaussian jumps)
12. Resetting Brownian motion
13. Returning random walk on a lattice
14. Territorial random walk

If you use this code, please cite the most appropiate of the below listed publications.

## Publications

* [*Mean perimeter and area of the convex hull of a planar Brownian motion in the presence of resetting*, Satya N. Majumdar, Francesco Mori, Hendrik Schawe, Grégory Schehr, Physical Review E 103, 022135 (2021)](https://dx.doi.org/10.1103/PhysRevE.103.022135)
* [*Position distribution in a generalized run-and-tumble process*, David S. Dean, Satya N. Majumdar, Hendrik Schawe, Physical Review E 103, 012130 (2021)](https://dx.doi.org/10.1103/PhysRevE.103.012130)
* [*Large deviations of a random walk model with emerging territories*, Hendrik Schawe, Alexander K. Hartmann, Physical Review E 102, 062141 (2020)](https://dx.doi.org/10.1103/PhysRevE.102.062141)
* [*The convex hull of the run-and-tumble particle in a plane*, Alexander K Hartmann, Satya N Majumdar, Hendrik Schawe, Grégory Schehr, Journal of Statistical Mechanics: Theory and Experiment 2020, 053401 (2020)](https://dx.doi.org/10.1088/1742-5468/ab7c5f)
* [*Large Deviations of Convex Hulls of the "True" Self-Avoiding Random Walk*, Hendrik Schawe, Alexander K Hartmann, Journal of Physics: Conference Series 1290, 012029 (2019)](https://dx.doi.org/10.1088/1742-6596/1290/1/012029)
* [*Large deviations of convex hulls of self-avoiding random walks*, Hendrik Schawe, Alexander K. Hartmann, Satya N. Majumdar, Physical Review E 97, 062159 (2018)](https://dx.doi.org/10.1103/PhysRevE.97.062159)
* [*Convex hulls of random walks in higher dimensions: A large-deviation study*, Hendrik Schawe, Alexander K. Hartmann, Satya N. Majumdar, Physical Review E 96, 062101 (2017)](https://dx.doi.org/10.1103/PhysRevE.96.062101)

## :hammer: Initialization

To clone the needed libraries, like qhull and tclap run:

```bash
git submodule update --init --recursive
```


## :books: Documentatation

Doxygen documentation of the latest version is at [surt91.github.io/randomWalk](https://surt91.github.io/randomWalk).


## :herb: Dependencies

* C++11 compatible compiler (e.g. gcc >=4.8.1)
* python3
    * numpy
    * scipy
* gnuplot


## :runner: Usage

1. Compile and run test with a call to `make` from the root directory.
2. Change the `./py/parameters.py` to the wanted values.
3. Change into `./py`, call `python3 call.py` and wait for it to finish.
   This will create the "rawData".
4. `python3 evaluate.py` will evaluate the results to intermediate "data".
    And `python3 gnuplot.py` will visualize this data with plots in `py/plots`.

### Misc

To compile with clang use:

```bash
CC=clang CXX=clang++ make
```

Activate multithreading for Wang Landau and Parallel Tempering with

```bash
OMP=1 make
```

To enable a MPI based Parallel Tempering method (m=5) (Tested with OpenMPI),
compile with

```bash
MPI=1 make
```

## :test_tube: Tests

To compile and run the tests, just `make testD`.

## :robot: Benchmark

To compile and run the benchmarks, just `make bench`.
This needs cmake in order to compile Googles benchmark library.
