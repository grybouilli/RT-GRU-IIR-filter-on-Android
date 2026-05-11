import numpy as np
import os


def load_files(
    files: list[str],
    eps_order: list[str] = [
        "CPU",
        "XNNPACK",
        "qnn-cpu",
        "WebGPU",
        "qnn-gpu",
        "qnn-htp",
    ],
    warm_up_buffers: int = 10,
) -> dict:
    eps = {}

    for file in files:
        data = np.load(file)
        ep = os.path.basename(file).split("_")[0]
        if ep in eps:
            eps[ep] = np.concat([eps[ep], data[warm_up_buffers:]])
        else:
            eps[ep] = data[warm_up_buffers:]

    return dict((k, eps[k]) for k in eps_order)
