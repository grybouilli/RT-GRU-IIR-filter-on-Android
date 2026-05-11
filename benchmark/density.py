import matplotlib.pyplot as plt
from argparse import ArgumentParser
import utils as ut
import numpy as np
from scipy.stats import gaussian_kde
from matplotlib.lines import Line2D

parser = ArgumentParser()

parser.add_argument("--files", nargs="+", default=["latency.npy"])
parser.add_argument("--buffer_size", type=int, default=256)
parser.add_argument("--sample_rate", type=int, default=48000)

args = parser.parse_args()

colors = ["black", "black", "black", "blue", "blue", "red"]
ordered_eps = ut.load_files(args.files)

ep_amount = len(ordered_eps.keys())

rows = 2
cols = ep_amount // 2
fig, axs = plt.subplots(nrows=rows, ncols=cols)

print(f"rows = {rows} cols = {cols}")
i, j = 0, 0
idx = 0
for ep, data in ordered_eps.items():
    print(f"plot {i} {j}")
    ax = axs[j, i]
    ax.hist(data, label="ep", color=colors[idx], bins="auto", density=True, alpha=0.45)

    kde = gaussian_kde(data, bw_method=0.12)
    x = np.linspace(data.min(), data.max(), len(data))
    ax.plot(x, kde(x), color=colors[idx], label=ep)
    ax.set_title(f"{ep}")
    ax.set_xlabel("latency (ms)")
    ax.set_ylabel("Probability of occurence")

    print(f"plot {i} {j} done")
    i = (i + 1) % cols
    j = j + 1 if i == 0 else j
    idx += 1

custom_lines = [
    Line2D([0], [0], color=colors[0], lw=4),
    Line2D([0], [0], color=colors[3], lw=4),
    Line2D([0], [0], color=colors[-1], lw=4),
]
fig.legend(
    custom_lines,
    ["CPU", "GPU", "DSP"],
    title="HW used for inference",
    loc="lower center",
    ncol=cols,  # 3 columns to display side by side (CPU, GPU, DSP)
    frameon=True,
)
fig.suptitle("Latency densities")
plt.subplots_adjust(bottom=0.2)  # increase if legend gets clipped
plt.tight_layout(rect=[0, 0.08, 1, 1])
plt.show()
