#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
from subprocess import call
from multiprocessing import Pool

import jinja2


class Gnuplot():
    def __init__(self, **kwargs):
        self.kwargs = kwargs

        self.env = jinja2.Environment(trim_blocks=True,
                                      lstrip_blocks=True,
                                      loader=jinja2.FileSystemLoader("templates"))
        self.env.globals.update(zip=zip)

        self.d = "plots"
        self.dataPath = "../data"
        self.rawDataPath = "../rawData"

        self.observable = "A" if kwargs["observable"] == 2 else "L"

        os.makedirs(self.d, exist_ok=True)

    def every(self):
        if self.kwargs["sampling"] == 1:
            self.create("rawData", "{/Italic t}", "{/Symbol %s}" % self.observable)
            self.create("rawHisto", "{/Symbol %s}" % self.observable, "{/Italic count}")
            self.create("unstiched", "{/Symbol %s}" % self.observable, "{/Italic count}")
            self.create("stiched", "{/Symbol %s}" % self.observable, "{/Italic p}")
            self.create("scaled", "{/Symbol %s} / {/Italic T%s}" % (self.observable, "" if self.observable == "A" else "^{1/2}"), "{/Italic T%s p}" % ("" if self.observable == "A" else "^{1/2}"))
            self.create("whole_distribution", "{/Symbol %s}" % self.observable, "{/Italic p}")
            self.create("r", "{/Italic r}", "{/Italic N}")
            self.create("r2", "{/Italic r^2}", "{/Italic N}")
            self.create("Z", "{/Italic %s}" % self.observable, "ln({/Italic Z}(theta_i)) ratios minus their mean")
        elif self.kwargs["sampling"] == 2:
            self.create("wl", "{/Symbol %s}" % self.observable, "{/Italic p}")
            self.create("wl_scaled", "{/Symbol %s} / {/Italic T%s}" % (self.observable, "" if self.observable == "A" else "^{1/2}"), "{/Italic T%s p}" % ("" if self.observable == "A" else "^{1/2}"))
            self.create("wl_raw", "{/Symbol %s}" % self.observable, "{/Italic counts}")
            self.create("wl_stiched", "{/Symbol %s}" % self.observable, "{/Italic counts}")

    def create(self, name="something", xl="", yl="", **kwargs):
        template = self.env.get_template(name+".gp")

        with open(os.path.join(self.d, name+".gp"), "w") as f:
            f.write(template.render(xlabel=xl,
                                    ylabel=yl,
                                    path=self.dataPath,
                                    raw=self.rawDataPath,
                                    filename=name,
                                    **self.kwargs,
                                    **kwargs))

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
