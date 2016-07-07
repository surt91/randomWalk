#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
from subprocess import call
from multiprocessing import Pool

import jinja2

from data import nu
from commonEvaluation import getMaximumSForGnuplot


def makebase(name, **kwargs):
    return name.format(**kwargs)


class Gnuplot():
    def __init__(self, **kwargs):
        self.kwargs = kwargs

        self.env = jinja2.Environment(trim_blocks=True,
                                      lstrip_blocks=True,
                                      loader=jinja2.FileSystemLoader("templates"))
        self.env.globals.update(zip=zip)
        self.env.globals.update(enumerate=enumerate)
        self.env.globals.update(makebase=lambda *x, **y: makebase(*x, **y, **self.kwargs))
        self.env.globals.update(getMaximumSForGnuplot=getMaximumSForGnuplot)

        self.d = "plots"
        self.dataPath = "../data"
        self.rawDataPath = "../rawData"

        self.observable = "A" if kwargs["observable"] == 2 else "L"

        os.makedirs(self.d, exist_ok=True)

    def every(self):
        fmt = "%f*d" if self.kwargs["observable"] == 2 else "%f*(d-1)"
        exponent = fmt % nu[self.kwargs["typ"]][self.kwargs["dimension"]]
        if self.kwargs["sampling"] == 1 or self.kwargs["sampling"] == 4:
            self.create("rawData", "{/Italic t}", "{/Symbol %s}" % self.observable)
            self.create("rawHisto", "{/Symbol %s}" % self.observable, "{/Italic count}")
            self.create("unstiched", "{/Symbol %s}" % self.observable, "{/Italic count}")
            self.create("stiched", "{/Symbol %s}" % self.observable, "{/Italic p}")
            self.create("scaled", "{/Symbol %s} {/Italic T^{%s}}" % (self.observable, exponent), "{/Italic T^{%s} p}" % exponent)
            self.create("whole_distribution", "{/Symbol %s}" % self.observable, "{/Italic p}")
            self.create("r", "{/Italic N}", "{/Italic r}")
            self.create("r2", "{/Italic r^2}", "{/Italic N}")
            self.create("Z", "{/Italic %s}" % self.observable, "ln({/Italic Z}(theta_i)) ratios minus their mean")
            self.create("rate_function", "{/Symbol %s}" % self.observable, "{/Symbol F}")
        elif self.kwargs["sampling"] == 2 or self.kwargs["sampling"] == 3:
            self.create("wl", "{/Symbol %s}" % self.observable, "{/Italic p}")
            self.create("wl_scaled", "{/Symbol %s} {/Italic T^{%s}}" % (self.observable, exponent), "{/Italic T^{%s} p}" % exponent)
            self.create("wl_raw", "{/Symbol %s}" % self.observable, "{/Italic counts}")
            self.create("wl_stiched", "{/Symbol %s}" % self.observable, "{/Italic counts}")
            self.create("wl_rate_function", "{/Symbol %s}" % self.observable, "{/Symbol F}")

        self.create("wl_rate_function_tran", "{/Symbol %s}" % self.observable, "{/Symbol F}")
        self.create("max", "N", "max pos")

    def create(self, name="something", xl="", yl="", **kwargs):
        template = self.env.get_template(name+".gp")

        with open(os.path.join(self.d, name+".gp"), "w") as f:
            f.write(template.render(xlabel=xl,
                                    ylabel=yl,
                                    path=self.dataPath,
                                    raw=self.rawDataPath,
                                    filename=name,
                                    param=self.kwargs,
                                    nu=nu,
                                    **self.kwargs,
                                    **kwargs))

    @staticmethod
    def plotall(png=False):
        os.chdir("plots")
        p = Pool()

        plots = []
        for i in os.listdir("."):
            if ".gp" in i:
                plots.append(i)
        cmds = [["gnuplot", i] for i in plots]
        print("gnuplot")
        p.map(silentCall, cmds, 1)

        if png:
            plots = []
            for i in os.listdir("."):
                if ".eps" in i:
                    plots.append(i)
            cmds = [["inkscape", "-z", "-b", '"#fff"', "-e", i.replace(".eps", ".png").strip(), "-h", "1080", i] for i in plots]
            print("inkscape")
            p.map(silentCall, cmds, 1)

        os.chdir("..")


def silentCall(x):
    call(x, stdout=open(os.devnull, 'wb'), stderr=open(os.devnull, 'wb'))


def main():
    import parameters as para

    g = Gnuplot(**para.parameters)
    print("generate Gnuplot files from Templates")
    g.every()
    g.plotall("--png" in sys.argv)


if __name__ == "__main__":
    main()
