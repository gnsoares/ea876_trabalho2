import matplotlib.pyplot as plt
import numpy as np
import os
import sys
import time


if __name__ == '__main__':

    REPETITIONS = 100
    Ns = np.array([2, 3, 5])
    METHODS = {
        'synchronous': {
            'call': 'sync',
            'results': {N: [] for N in Ns},
        },
        'multiprocess': {
            'call': 'proc',
            'results': {N: [] for N in Ns},
        },
        'multithread': {
            'call': 'thrd',
            'results': {N: [] for N in Ns},
        },
    }

    for _ in range(REPETITIONS):
        for N in Ns:
            for method in METHODS:

                start = time.time()
                call = '{exe} {file} {N} {method}'
                os.popen(call.format(
                    exe=sys.argv[1],
                    file=sys.argv[2],
                    N=N,
                    method=METHODS[method]['call'],
                )).read()
                end = time.time()
                METHODS[method]['results'][N].append(1000*(end - start))

    labels = Ns
    means = [
        [np.mean(METHODS[m]['results'][N]) for N in labels] for m in METHODS
    ]
    err = [
        [np.std(METHODS[m]['results'][N]) for N in labels] for m in METHODS
    ]

    x = np.arange(len(labels))
    width = 0.35
    locs = [-.5, 0, .5]

    fig, ax = plt.subplots(figsize=(10.5, 4.5))
    for i, m in enumerate(METHODS):
        ax.bar(
            x + locs[i]*width,
            means[i],
            width/2,
            label=m,
            yerr=err[i],
            capsize=5,
            zorder=3
        )

    ax.set_title('Time elapsed to blur image with different '
                 'intensity and methods', fontsize=14)
    ax.set_ylabel('Time (ms)', fontsize=14)
    ax.set_xlabel('N', fontsize=14)
    ax.set_xticks(x)
    ax.set_xticklabels(labels, fontsize=14)
    ax.grid(axis='y', zorder=0)
    ax.legend(loc=4, fontsize=14)

    fig.tight_layout()
    fig.savefig('figure.pdf')
