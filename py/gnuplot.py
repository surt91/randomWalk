#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
from subprocess import call
from multiprocessing import Pool

import jinja2


class Gnuplot():
    def __init__(self, number_of_steps, typ, seedMC, seedR, iterations,
                       dimension, thetas, directory, rawData, rawConf,
                       observable, method, akl):
        self.N = number_of_steps
        self.T = thetas
        self.iterations = iterations
        self.seedMC = seedMC
        self.seedR = seedR
        self.typ = typ
        self.env = jinja2.Environment(trim_blocks=True,
                                      lstrip_blocks=True,
                                      loader=jinja2.FileSystemLoader("templates"))
        self.env.globals.update(zip=zip)

        self.d = "plots"
        self.dataPath = "../data"
        self.rawData = rawData
        if not os.path.exists(self.d):
            os.makedirs(self.d)

    def every(self):
        pass

    def create(self, name="something", xl="", yl=""):
        template = self.env.get_template(name+".gp")

        with open(os.path.join(self.d, name+".gp"), "w") as f:
            f.write(template.render(N=self.N,
                                    T=self.T,
                                    typ=self.typ,
                                    iterations=self.iterations,
                                    seedMC=self.seedMC,
                                    seedR=self.seedR,
                                    path=self.dataPath,
                                    rawPath=self.rawData,
                                    filename=name,
                                    xlabel=xl,
                                    ylabel=yl))

    @staticmethod
    def plotall():
        os.chdir("plots")
        p = Pool()

        plots = []
        for i in os.listdir("."):
            if ".gp" in i:
                plots.append(i)
        cmds = [["gnuplot", i] for i in plots]
        p.map(silentCall, cmds, 1)

        plots = []
        for i in os.listdir("."):
            if ".eps" in i:
                plots.append(i)
        cmds = [["inkscape", "-z", "-b", '"#fff"', "-e", i.replace(".eps", ".png").strip(), "-h", "1080", i] for i in plots]
        p.map(silentCall, cmds, 1)

        os.chdir("..")


def silentCall(x):
    call(x, stdout=open(os.devnull, 'wb'), stderr=open(os.devnull, 'wb'))


def main():
    import parameters as para

    g = Gnuplot(**para.parameters)
    print("generate Gnuplot files from Templates")
    g.every()
    print("gnuplot and inkscape")
    g.plotall()


if __name__ == "__main__":
    main()
