import os

import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

import parameters as param

os.makedirs("plots", exist_ok=True)

for t1 in param.parameters["passageTimeStart"]:
    a = np.loadtxt("rawData/{}.dat.gz".format(param.basetheta.format(t1=t1, steps=param.parameters["number_of_steps"][0], theta=float("inf"), **param.parameters)))
    a = a.transpose()
    b = a[1][a[1] >= 0]

    total = len(a[1])
    evenlonger = len(a[1][a[1] < 0])

    plt.title("t1 = {}".format(t1))
    plt.xlabel("t2")
    plt.ylabel("count")

    y, x, _ = plt.hist(b, 50, log=True)
    with open("plots/t1{}.dat".format(t1), "w") as f:
        for l in zip(x, y):
            f.write("{} {}\n".format(*l))
    plt.savefig("plots/t1{}.png".format(t1))
    plt.clf()
    
    x = []
    y = []
    for r in np.linspace(0, 1, 100):
        t2 = t1//r
        # b contains positions of the first occurence of a different sign
        # to get the last with the same sign (or zero) -> gt instead of ge
        q = (len(b[b > t2]) + evenlonger) / total
        x.append(t1/t2)
        y.append(q)

    plt.title("t1 = {}".format(t1))
    plt.xlabel("t1/t2")
    plt.ylabel("Q")

    plt.plot(x, y, "+")
    x = np.linspace(0, 1, 100);
    y = 2/np.pi*np.arcsin(np.sqrt(x))
    plt.plot(x, y, label="$2/\pi \arcsin(\sqrt(t1/t2))$")
    plt.savefig("plots/q{}.png".format(t1))
    plt.clf()
