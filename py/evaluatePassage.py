import os
import math

import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

import parameters as param
from evaluateMetropolis import getAutocorrTime

os.makedirs("plots", exist_ok=True)

t1s = param.parameters["passageTimeStart"][0]

for N in param.parameters["number_of_steps"]:
    for n, t1 in enumerate(t1s):
        a = np.loadtxt("rawData/{}.dat.gz".format(param.basetheta.format(t1=t1, steps=N, theta=float("inf"), **param.parameters)))
        a = a.transpose()
        a = a[(-len(t1s))+n]
        t_eq = 1000
        a = a[t_eq:]
        b = a[a >= 0]

        # if a markov chain algorithm was used, discard correlated samples
#        if param.parameters["sampling"] == 1:
#            if len(b) > 100:
#                tau = math.ceil(getAutocorrTime(b))
#            a = a[::2*tau]
#            b = a[a >= 0]

        total = len(a)
        evenlonger = len(a[a < 0])

#        plt.title("t1 = {}".format(t1))
#        plt.xlabel("t2")
#        plt.ylabel("count")

#        y, x, _ = plt.hist(b, 50)
#        with open("plots/t1_{}_N{}.dat".format(t1, N), "w") as f:
#            for l in zip(x, y):
#                f.write("{} {}\n".format(*l))
#        plt.savefig("plots/t1_{}_N{}.png".format(t1, N))
#        plt.clf()

        with open("plots/q_{}_N{}.dat".format(t1, N), "w") as f:
            f.write("# t1 t2 Q\n")
            x = []
            y = []
            for r in np.logspace(-4, 0, 100, base=10):
                t2 = t1//r
                if t2 > N:
                    continue
                # b contains positions of the first occurence of a different sign
                # to get the last with the same sign (or zero) -> gt instead of ge
                q = (len(b[b > t2]) + evenlonger) / total
                x.append(t1/t2)
                y.append(q)

                f.write("{} {} {}\n".format(t1, t2, q))

        plt.title("t1 = {}".format(t1))
        plt.xlabel("t1/t2")
        plt.ylabel("Q")

        plt.plot(x, y, "+")

        x = np.linspace(0, 1, 100);
        y = 2/np.pi*np.arcsin(np.sqrt(x))
        plt.plot(x, y, label="$2/\pi \arcsin(\sqrt(t1/t2))$")
        plt.savefig("plots/q_{}_N{}.png".format(t1, N))
        plt.clf()
