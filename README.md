randomWalk
==========

[![Build Status](https://travis-ci.com/surt91/randomWalk.svg?token=KcmDorpEqtSzJp2wyhgU&branch=master)](https://travis-ci.com/surt91/randomWalk)
[![codecov](https://codecov.io/gh/surt91/randomWalk/branch/master/graph/badge.svg?token=EW5j2uy4vc)](https://codecov.io/gh/surt91/randomWalk)

## Initialization

To clone the needed libraries, like qhull and tclap run:

    git submodule update --init --recursive


## Documentatation

Doxygen documentation of the latest version is at [surt91.github.io/randomWalk](https://surt91.github.io/randomWalk).


## Dependencies

* C++11 compatible compiler (e.g. gcc >=4.8.1)
* python3
    * numpy
    * scipy
* gnuplot


## Usage

1. Compile and run test with a call to `make` from the root directory.
2. Change the `./py/parameters.py` to the wanted values.
3. Change into `./py`, call `python3 call.py` and wait for it to finish.
   This will create the "rawData".
4. `python3 evaluate.py` will evaluate the results to intermediate "data".
    And `python3 gnuplot.py` will visualize this data with plots in `py/plots`.

## Tests

To compile and run the tests, just `make testD`.

## Benchmark

To compile and run the benchmarks, just `make bench`.
This needs cmake in order to compile Googles benchmark library.

## Misc

To compile with clang use:

    CC=clang CXX=clang++ make

Activate multithreading for Wang Landau and Parallel Tempering with

    OMP=1 make

To enable a MPI based Parallel Tempering method (m=5) (Tested with OpenMPI),
compile with

    MPI=1 make
