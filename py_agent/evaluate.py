import os
import shutil
from subprocess import call

from parameters import parameters


for p in parameters:
    filename = p["file"].format(T="merged", **p)

    if p["sampling"] == 1:
        if not os.path.exists("glue++"):
            try:
                shutil.copyfile("../glue++/glue++", ".")
            except OSError:
                print("can not copy glue++")

        if p["observable"] == 1:
            max_observable = 4 * p["sidelength"]
        else:
            max_observable = p["sidelength"] ** 2

        cmd = [
            "./glue++",
            "-B {}".format(max_observable),
            "-l {}".format(0),
            "-u {}".format(max_observable),
            "-o {}".format(filename + ".dat"),
            # just skip 10% for equilibration
            "-s {}".format(p["iterations"] * 0.1),
            "-v 5",
        ]

        for T in p["temperatures"]:
            infile = "rawData/" + p["file"].format(T=T, **p) + ".dat.gz"
            cmd += ["-T {}".format(T),
                    "-i {}".format(infile)]

        try:
            call(cmd)
        except PermissionError:
            print("glue++ is not executable, change that or run:")
            print(" ".join(cmd))
