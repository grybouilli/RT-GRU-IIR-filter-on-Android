import matplotlib.pyplot as plt
import numpy as np
from argparse import ArgumentParser
import os
import utils as ut

parser = ArgumentParser()

parser.add_argument("--files", nargs="+", default=["latency.npy"])
parser.add_argument("--buffer_size", type=int, default=256)
parser.add_argument("--sample_rate", type=int, default=48000)

args = parser.parse_args()

warm_up_buffers = 10
eps = {}

eps_order = ["CPU", "XNNPACK", "qnn-cpu", "WebGPU", "qnn-gpu", "qnn-htp"]
colors = ["black", "black", "black", "blue", "blue", "red"]

ordered_eps = ut.load_files(
    args.files, eps_order=eps_order, warm_up_buffers=warm_up_buffers
)

plt.figure()
ax = plt.boxplot(
    [d for _, d in ordered_eps.items()],
    whis=(0, 100),
    labels=[key for key in ordered_eps.keys()],
    patch_artist=True,
)

for i, color in enumerate(colors):
    ax["boxes"][i].set(color=color, facecolor="white")
    ax["caps"][i * 2].set(color=color)
    ax["caps"][i * 2 + 1].set(color=color)
    ax["whiskers"][i * 2].set(color=color)
    ax["whiskers"][i * 2 + 1].set(color=color)
    ax["fliers"][i].set(color=color, markeredgecolor=color)
    ax["medians"][i].set(color="orange")

plt.ylabel("Latency per inference (ms)")

plt.show()
