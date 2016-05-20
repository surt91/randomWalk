import gzip
import logging

def getMinMaxTime(filenames):
    times = []
    mems = []
    for filename in filenames:
        with gzip.open(filename+".gz", "rt") as f:
            for i in f.readlines():
                if "# Does not equilibrate" in i:
                    break
                if "# time in seconds" in i or "# time/sweep in seconds" in i:
                    s = i.split(":")[-1].strip().strip("s")
                    times.append(float(s))
                if "# max vmem: VmPeak" in i:
                    s = i.split(":")[-1].strip().strip(" kB")
                    mems.append(float(s))
    try:
        logging.info("time/sweep between {:.2f}s - {:.2f}s".format(min(times), max(times)+0.0051))
    except ValueError:
        logging.info("No time measured")
    try:
        logging.info("memory between {:.0f}kB - {:.0f}kB".format(min(mems), max(mems)))
    except ValueError:
        logging.info("No memory measured")
