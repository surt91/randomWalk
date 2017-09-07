from math import gamma
from itertools import product
from time import clock


def candidateCombinations(T, d):
    if d == 2:
        for i in range(1, T + 1):
            for j in range(1, T + 1 - i):
                yield (i, j)
    elif d == 3:
        for i in range(1, T + 1):
            for j in range(1, T + 1 - i):
                for k in range(1, T + 1 - i - j):
                    yield (i, j, k)
    elif d == 4:
        for i in range(1, T + 1):
            for j in range(1, T + 1 - i):
                for k in range(1, T + 1 - i - j):
                    for l in range(1, T + 1 - i - j - k):
                        yield (i, j, k, l)
    elif d == 5:
        for i in range(1, T + 1):
            for j in range(1, T + 1 - i):
                for k in range(1, T + 1 - i - j):
                    for l in range(1, T + 1 - i - j - k):
                        for m in range(1, T + 1 - i - j - k - l):
                            yield (i, j, k, l, m)
    elif d == 6:
        for i in range(1, T + 1):
            for j in range(1, T + 1 - i):
                for k in range(1, T + 1 - i - j):
                    for l in range(1, T + 1 - i - j - k):
                        for m in range(1, T + 1 - i - j - k - l):
                            for n in range(1, T + 1 - i - j - k - l - m):
                                yield (i, j, k, l, m, n)
    else:
        for c in product(range(1, T + 1), repeat=d):
            yield c


def sumOfCombinations(T, d):
    return sum(productOfCombinations(c, T) for c in candidateCombinations(T, d))


def productOfCombinations(combination, bound):
    p = 1
    if sum(combination) > bound:
        return 0
    for j in combination:
        p *= j
    return 1 / p**0.5


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
