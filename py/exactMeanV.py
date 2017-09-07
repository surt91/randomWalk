from math import gamma
from itertools import product
from time import clock


def sumOfCombinations(T, d):
    s = 0
    if d == 2:
        for i in range(1, T + 1):
            for j in range(1, T + 1 - i):
                s += (i*j)**-0.5
    elif d == 3:
        for i in range(1, T + 1):
            for j in range(1, T + 1 - i):
                for k in range(1, T + 1 - i - j):
                    s += (i*j*k)**-0.5
    elif d == 4:
        for i in range(1, T + 1):
            for j in range(1, T + 1 - i):
                for k in range(1, T + 1 - i - j):
                    for l in range(1, T + 1 - i - j - k):
                        s += (i*j*k*l)**-0.5
    elif d == 5:
        for i in range(1, T + 1):
            for j in range(1, T + 1 - i):
                for k in range(1, T + 1 - i - j):
                    for l in range(1, T + 1 - i - j - k):
                        for m in range(1, T + 1 - i - j - k - l):
                            s += (i*j*k*l*m)**-0.5
    elif d == 6:
        for i in range(1, T + 1):
            for j in range(1, T + 1 - i):
                for k in range(1, T + 1 - i - j):
                    for l in range(1, T + 1 - i - j - k):
                        for m in range(1, T + 1 - i - j - k - l):
                            for n in range(1, T + 1 - i - j - k - l - m):
                                s += (i*j*k*l*m*n)**-0.5
    else:
        for c in product(range(1, T + 1), repeat=d):
            if sum(c) <= T:
                p = 1
                for i in c:
                    p *= i

                s += p**-0.5
    return s


def meanV(T, d):
    return 2**(-d / 2) / gamma(d / 2 + 1) * sumOfCombinations(T, d)


if __name__ == '__main__':
    print("# T d meanV time(s)")
    for d in (2, 3, 4, 5, 6):
        for T in (181, 362, 724, 1448, 2896, 5792, 11586, 23170, 46340, 92682, 185364):
            s = clock()
            result = meanV(T, d)
            e = clock()
            time = "{:.3f}".format(e - s)
            print(T, d, result, time)
