#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from subprocess import call
from multiprocessing import Pool
import time
import logging

from config import Simulation
import gnuplot
from gnuplot import Gnuplot
import parameters as para

logging.basicConfig(level=logging.INFO,
                format='%(asctime)s -- %(levelname)s :: %(message)s',
                datefmt='%d.%m.%YT%H:%M:%S')
logging.info("started")


def read_parameters():
    g = Gnuplot(**para.parameters)
    g.every()
    return Simulation(**para.parameters)


if __name__ == "__main__":
    run = read_parameters()

    if len(sys.argv) > 1:
        if "hero" in sys.argv:
            run.hero()
        elif "plot" in sys.argv:
            print("only plot")
            gnuplot.main()
        else:
            print("."+sys.argv[1]+".")
            print("only known options: hero, plot")
    else:
        run()
