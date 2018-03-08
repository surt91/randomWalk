import os
import sys
from subprocess import call
from multiprocessing import Pool

from parameters import parameters


os.makedirs("rawData", exist_ok=True)

cmds = []
for p in parameters:
    for T in p["temperatures"]:
        cmd = ["./randomWalk"]
        cmd += ["-T", f"{T}"]
        # only agent walk
        cmd += ["-t", 9]

        n = p["iterations"]
        cmd += ["-n", f"{n}"]

        N = p["Tstar"]
        cmd += ["-N", f"{N}"]

        Tas = p["Tas"]
        cmd += ["--Tas", f"{Tas}"]

        M = p["agents"]
        cmd += ["-M", f"{M}"]

        width = p["sidelength"]
        cmd += ["--width", f"{width}"]

        w = p["observable"]
        cmd += ["-w", f"{w}"]

        outfile = "rawData/" + p["file"].format(N=N, T=T, **p) + ".dat"
        cmd += ["-o", outfile]

        # if there is already a finished output, do not simulate again
        if not os.path.exists(outfile + ".gz"):
            cmds.append(cmd)

if "hpc" in sys.argv:
    with open("jobs.lst", "w") as f:
        for cmd in cmds:
            f.write("{}\n".format(" ".join(cmd)))
else:
    with Pool() as P:
        P.map(call, cmds)
