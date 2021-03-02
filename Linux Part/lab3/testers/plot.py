#!python
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import sys
from collections import defaultdict

def plotdf(ax, df, label, threshold=float('inf')):
    data = defaultdict(list)
    for idx, row in df.iterrows():
        data[int(row['cnt_p'])].append(row['time'])
    for p in data:
        data[p] = np.array(data[p])
    # print(data)
    # plot all data points
    xall, yall = [], []
    # plot average data line and error bars
    xavg, yavg, err = [], [], []
    for idx, nparr in data.items():
        # print(idx, nparr)
        xall += [idx] * len(nparr)
        yall += list(nparr)
        xavg.append(idx)
        yavg.append(nparr.mean())
        err.append(nparr.std())
        # top threshold
        if yavg[-1] > threshold:
            break
    ax.errorbar(xavg, yavg, yerr=err, capsize=0, label=label, linewidth=2)
    ax.scatter(xall, yall, 10, c='black', alpha=0.05)

def plotproc(ax, procname, title=None, suffix='', threshold=float('inf')):
    folder = procname + '_data' + suffix
    # folder = 'data_4cores'
    locks = ['pthread_mutex', 'spinlock', 'my_mutex', 'pthread_spinlock']
    # locks = ['pthread_mutex', 'spinlock', 'my_mutex']
    paths = []

    # compare performance of different locks
    for lockname in locks:
        paths.append('../data/%s/%s_%s.csv' % (folder, procname, lockname))

    # threshold = 13
    for filepath, lockname in zip(paths, locks):
        print(filepath)
        df = pd.read_csv(filepath)
        # print(df)
        plotdf(ax, df, lockname, threshold)
    ax.set_xlabel('pthread numbers')
    ax.set_ylabel('time')
    ax.set_title(title)
    ax.legend()
    
# fig, (ax1, ax2, ax3) = plt.subplots(1, 3, sharex=True, sharey=True)
# fig, (ax1, ax2) = plt.subplots(1, 2, sharex=True, sharey=True)
fig, ax = plt.subplots()
# threshold = float('inf')
# plotproc(ax1, 'lti', 'list insert')
# plotproc(ax2, 'ct', 'counter increment')
# plotproc(ax1, 'ltd', '$O(1) delete$')
# plotproc(ax2, 'ltdon', '$O(n) delete$')
plotproc(ax, 'hts', 'hash table scaling test', '')
# plotproc(ax1, 'hti', 'each list has independent lock')
# plotproc(ax2, 'hflti', 'hash table with one whole lock')
# plotproc(ax1, 'ct', '2 cores VMWare', '_2cores')
# plotproc(ax2, 'ct', '1 core VMWare', '_1core')
# plotproc(ax3, 'ct', '2 cores server', '_2cores_real')

# plt.xlabel('pthread numbers')
# plt.ylabel('time')
# fig.suptitle('counter increment tests on different environment')
plt.show()