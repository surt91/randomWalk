iterations = 10**4
basename = "out_L{sidelength}_M{agents}_T{T}_Tas{Tas}_Tstar{Tstar}_n{iterations}_w{observable}"

default = {
    # MC Sweeps
    "iterations": iterations,
    # 0: simple, 1: metropolis, 2: WL, ...
    "sampling": 1,
    # number of agents
    "agents": 10,
    # active scent time
    "Tas": 800,
    # time for development
    "Tstar": 2000,
    # name of output, N and T are size and temperature
    "file": basename,

    # temperatures (only metropolis)
    "temperatures": [10000],
    # size of the field
    "sidelength": None,
    # 0: perimeter, 1: area
    "observable": 1
}

details = [
    {
        # temperatures (only metropolis)
        "temperatures": [0.1, 1, 10, 100, 10000, -10, -1, -0.1],
        # size of the field
        "sidelength": 10,
    },
    {
        # temperatures (only metropolis)
        "temperatures": [0.1, 1, 10, 100, 10000, -10, -1, -0.1],
        # size of the field
        "sidelength": 20,
    },
    {
        # temperatures (only metropolis)
        "temperatures": [0.1, 1, 10, 100, 10000, -10, -1, -0.1],
        # size of the field
        "sidelength": 30,
    },
]


parameters = [dict(default, **d) for d in details]
