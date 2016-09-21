import os

import numpy as np
import matplotlib.pyplot as plt

os.makedirs("plots", exist_ok=True)

for i in range(0, 10+1):
    t1 = 2**i
    a = np.loadtxt("rawData/m1_t5_w3_d2_N4000_n10000_t1{}_Tinf.dat.gz".format(t1))
    a = a.transpose()
    b = a[1][a[1] >= 0]

    total = len(a[1])
    evenlonger = len(a[1][a[1] < 0])

    plt.hist(b, 50)
    plt.savefig("plots/t1{}.png".format(t1))
    plt.clf()
    
    x = []
    y = []
    for r in np.linspace(0, 1, 100):
        t2 = t1/r
        q = (len(b[b > t2]) + evenlonger) / total
        x.append(t1/t2)
        y.append(q)
    plt.plot(x, y)
    x = np.linspace(0, 1, 100);
    y = 2/np.pi*np.arcsin(np.sqrt(x))
    plt.plot(x, y)
    plt.savefig("plots/q{}.png".format(t1))
    plt.clf()
