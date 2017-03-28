import numpy as np
from scipy.interpolate import interp1d
from scipy.optimize import curve_fit
from matplotlib import pyplot as plt

def powerlaw(x, a, b, c):
    return a*x**b+c

d = {
    2048: "data/dist/combined_t5_w2_d4_N2048.dat",
    1024: "data/dist/WL_m3_t5_w2_d4_N1024_n4.dat",
    512: "data/dist/WL_m3_t5_w2_d4_N512_n4.dat",
    256: "data/dist/WL_m3_t5_w2_d4_N256_n4.dat",
    #128: "data/dist/WL_m3_t5_w2_d4_N128_n4.dat",
}

f = {}
nu = 0.5*4
for n, path in d.items():
    x, xerr, y, yerr = np.loadtxt(path, unpack=True)
    x /= n**nu
    y += np.log(n**nu)
    f[n] = interp1d(x, y)

pos = np.linspace(0.0015, 0.007, 30)

ns = sorted(d.keys())
p0 = (100., -0.3, -200.)
for p in pos:
    ys = []
    for n in ns:
        try:
            tmp = f[n](p)
        except ValueError:
            tmp = np.nan
        finally:
            ys.append(tmp)
    # print(p, ys)
    x = [n for n, y in zip(ns, ys) if not np.isnan(y)]
    y = [y for n, y in zip(ns, ys) if not np.isnan(y)]
    popt, pcov = curve_fit(powerlaw, x, y, p0=p0)

    # matplotlib plots
    plt.plot(x, y, "o")
    plt.plot(x, [powerlaw(p, *popt) for p in x])
    plt.savefig("out_{}.png".format(p))
    plt.clf()
    #print("#", popt)
    
    print(p, popt[2], pcov[2][2]**0.5)
