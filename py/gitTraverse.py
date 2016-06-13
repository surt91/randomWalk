"""This script visualizes the trend of the performance of the program.
"""

# path to the repo that should be examined
repo_path = "~/uni/Promotion/randomWalk"
# commit to start, empty string is the first commit
start_commit = ""
# commit to end, empty string is the last commit
end_commit = ""
# temporary path, where the repo is cloned to and benchmarks are done
tmp_path = "/tmp"
# test command, this command will be timed
cmd = "./randomWalk -t 1 -N 1000 -n 1000 -T -20 -a -c 2"

import os
import sys
import time
import binascii

import git

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
    os.system("make")
    start = time.time()
    e = os.system(cmd)
    duration = time.time() - start
    os.system("make clean")
    # program exits with errorcode -> do not use the measured time
    if e != 0:
        duration = float("nan")
    print(duration)
    times.append(duration)
    dates.append(c.committed_date)
    shas.append(sha)

    with open(os.path.join(pwd, "gitBenchmark.dat"), "a") as f:
        f.write('{} {} {} "{}"\n'.format(c.committed_date, sha, duration, c.message.split("\n")[0]))

for i, j, k in zip(dates, shas, times):
    print("{} {} {}".format(i, j, k))

print("plot in gnuplot with: \n"
      "set timefmt '%s'\n"
      "set format x '%m/%d/%Y %H:%M:%S'\n"
      "set xdata time\n"
      "p 'gitBench.dat' u 1:3 w l t '{}', '' u 1:3:4 with labels rotate left offset 0,char 1".format(cmd))
