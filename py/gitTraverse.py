"""This script visualizes the trend of the performance of the program.
"""

import os
import time
import binascii
from multiprocessing import cpu_count
from subprocess import STDOUT, check_call, CalledProcessError

import git

# path to the repo that should be examined
repo_path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
# commit to start, empty string is the first commit
start_commit = ""
# commit to end, empty string is the last commit
end_commit = ""
# temporary path, where the repo is cloned to and benchmarks are done
tmp_path = "/tmp"
# test command, this command will be timed
cmd = "./randomWalk -t 1 -N 1000 -n 1000 -T -20 -a -c 2"


if __name__ == "__main__":
    cloned_path = os.path.join(tmp_path, 'tmp_benchmark')
    repo = git.Repo(repo_path)
    try:
        cloned_repo = repo.clone(cloned_path)
    except git.exc.GitCommandError:
        pass

    pwd = os.getcwd()
    os.chdir(cloned_path)
    os.system("git checkout master")
    os.system("git submodule init")
    os.system("git submodule update")
    os.system("make clean")
    os.system("git pull origin master")
    os.chdir("src")
    os.system("make -j1")
    os.system("make clean")

    dates = []
    shas = []
    times = []
    with open(os.path.join(pwd, "gitBenchmark.dat"), "w") as f:
        f.write("# %s\n" % cmd)
        f.write("# unixtime commit benchmarktime message\n")

    N = -1
    for n, c in enumerate(repo.iter_commits('master', max_count=N)):
        # set HEAD to the current commit
        print("{}/{}".format(n, N))
        sha = binascii.hexlify(c.binsha).decode("ascii")
        print("git checkout {}".format(sha))
        os.system("git checkout {}".format(sha))
        os.system("make -j{}".format(cpu_count()))
        durations = []
        for i in range(10):
            start = time.time()
            try:
                check_call(cmd, stderr=STDOUT, timeout=60)  # abort after 1 minute
            except CalledProcessError:
                durations.append(60)
                break
            durations.append(time.time() - start)
        os.system("make clean")

        duration = sum(durations) / len(durations)
        duration_err = sum((i - duration)**2 for i in durations)**0.5 / len(durations)

        print(duration)
        times.append(duration)
        dates.append(c.committed_date)
        shas.append(sha)

        with open(os.path.join(pwd, "gitBenchmark.dat"), "a") as f:
            f.write('{} {} {} {} "{}"\n'.format(
                                                c.committed_date,
                                                sha,
                                                duration,
                                                duration_err,
                                                c.message.split("\n")[0])
                                               )

    for i, j, k in zip(dates, shas, times):
        print("{} {} {}".format(i, j, k))

    print("plot in gnuplot with:")
    print("set timefmt '%s'")
    print("set format x '%m/%d/%Y %H:%M:%S'")
    print("set xdata time")
    print("p 'gitBench.dat' u 1:3:4 w yerrorlines t '{}', '' u 1:3:5 with labels rotate left offset 0,char 1\n".format(c))
    print("or")
    print("p 'gitBench.dat' u 0:3:4 w yerrorlines t '{}', '' u 0:3:5 with labels rotate left offset 0,char 1".format(cm))
