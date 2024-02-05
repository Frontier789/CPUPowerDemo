# %%
from dataclasses import dataclass
import matplotlib.pyplot as plt
import numpy as np
from glob import glob

files = sorted(list(glob("V*.txt")))


@dataclass
class PlotEntry:
    Ns: list[float]
    gflops: list[float]
    name: str


entries = []

N_limit = 10000

for file in files:
    Ns = []
    gflops = []
    with open(file) as f:
        for line in f.readlines():
            for erase in ["N=", "GFLOPS=", "time=", "\n"]:
                line = line.replace(erase, "")
            values = [float(s) for s in line.split(", ")]
            assert len(values) == 3
            [N, gflop, time] = values
            if N < N_limit:
                Ns.append(N)
                gflops.append(gflop)
        entries.append(PlotEntry(Ns, gflops, file.replace(".txt", "")))


plot_until = 6

for e in entries[:plot_until]:
    plt.plot(e.Ns, e.gflops)
plt.legend([e.name for e in entries[:plot_until]])
plt.xlabel("N")
plt.ylabel("GFLOPs")

plt.show()


# %%
