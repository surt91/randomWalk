import os

import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

import parameters as param

os.makedirs("plots", exist_ok=True)

t1s = param.parameters["passageTimeStart"][0]

for N in param.parameters["number_of_steps"]:
    for n, t1 in enumerate(t1s):
        if param.parameters["sampling"] == 0:
            a = np.loadtxt("rawData/{}.dat.gz".format(param.basetheta.format(t1=t1, steps=N, theta=float("inf"), **param.parameters)))
            a = a.transpose()
            b1 = a[(-2*len(t1s))+n]
        else:
            print("only available for simple sampling")
            raise

        with open("plots/corr_{}_N{}.dat".format(t1, N), "w") as f:
            f.write("# t1 t2 corr\n")
            x = []
            y = []
            for m, t2 in enumerate([i for i in t1s if i > t1]):
                b2 = a[(-2*len(t1s))+n+1+m]
                x.append(t2)
                corr = np.mean([i1*i2 for i1, i2 in zip(b1, b2)]) - np.mean(b1)*np.mean(b2)
                # test normalization
                corr /= np.mean(np.abs(b1))*np.mean(np.abs(b2))
                y.append(corr)

                f.write("{} {} {}\n".format(t1, t2, corr))

        plt.title("t1 = {}".format(t1))
        plt.xlabel("t2")
        plt.ylabel("<x_t_1 x_t_2> - <x_t_1> <x_t_2>")

        plt.plot(x, y, "+")

        plt.savefig("plots/corr_{}_N{}.png".format(t1, N))
        plt.clf()
