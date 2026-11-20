#!/usr/bin/env python3
"""Analyze P09 1D/2D benchmark CSVs and generate reproducible plots.
Run from the P09FenwickTrees project root: python3 scripts/analyze_results.py
"""
import csv
import math
from collections import defaultdict
from pathlib import Path
from statistics import mean, stdev

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

RAW = Path('results/raw')
PROCESSED = Path('results/processed')
GRAPHS = Path('results/graphs')
METRICS = ('build_ms', 'update_ns', 'query_ns', 'total_ms', 'throughput_ops_sec')
LABELS = {
    'build_ms': 'Build time (ms)',
    'update_ns': 'Average update latency (ns)',
    'query_ns': 'Average query latency (ns)',
    'total_ms': 'Total benchmark time (ms)',
    'throughput_ops_sec': 'Throughput (operations/sec)',
}

def read_csv(dimension):
    path = RAW / f'benchmark_{dimension}.csv'
    with path.open(newline='') as f:
        rows = list(csv.DictReader(f))
    expected = {'algorithm', 'operations', 'update_percent', 'trial', 'checksum', *METRICS}
    expected.add('n' if dimension == '1d' else 'rows')
    if dimension == '2d':
        expected.add('cols')
    if not rows or not expected.issubset(rows[0]):
        raise ValueError(f'{path}: missing columns or empty CSV')
    for row in rows:
        for col in METRICS:
            row[col] = float(row[col])
        for col in ('operations', 'update_percent', 'trial'):
            row[col] = int(row[col])
        if dimension == '1d':
            row['size'] = str(int(row['n']))
        else:
            row['size'] = f"{int(row['rows'])}x{int(row['cols'])}"
    return rows

def summarize(rows, dimension):
    grouped = defaultdict(list)
    for row in rows:
        key = (row['size'], row['operations'], row['update_percent'], row['algorithm'])
        grouped[key].append(row)
    output = []
    for (size, operations, ratio, algo), samples in sorted(grouped.items()):
        result = dict(dimension=dimension, size=size, operations=operations,
                      update_percent=ratio, algorithm=algo, trials=len(samples))
        for metric in METRICS:
            values = [sample[metric] for sample in samples]
            result[f'{metric}_mean'] = mean(values)
            result[f'{metric}_std'] = stdev(values) if len(values) > 1 else 0.0
        output.append(result)
    return output

def write_summary(rows, dimension):
    output = PROCESSED / f'benchmark_{dimension}_summary.csv'
    with output.open('w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    return output

def plot_metric(summary, dimension, metric, ratio):
    selected = [r for r in summary if r['update_percent'] == ratio]
    sizes = sorted({r['size'] for r in selected}, key=lambda s: int(s.split('x')[0]))
    names = ['Fenwick', 'Segment'] if dimension == '1d' else ['Fenwick2D', 'Segment2D']
    fig, ax = plt.subplots(figsize=(8, 4.8))
    xs = list(range(len(sizes)))
    width = 0.36
    for i, algo in enumerate(names):
        lookup = {r['size']: r for r in selected if r['algorithm'] == algo}
        if len(lookup) != len(sizes):
            raise ValueError(f'Missing {algo} results for {dimension} {ratio}%')
        values = [lookup[size][f'{metric}_mean'] for size in sizes]
        deviations = [lookup[size][f'{metric}_std'] for size in sizes]
        offsets = [x + (i - 0.5) * width for x in xs]
        ax.bar(offsets, values, width, yerr=deviations, capsize=3, label=algo)
    ax.set_xticks(xs, sizes)
    ax.set_xlabel('Array size' if dimension == '1d' else 'Matrix dimensions')
    ax.set_ylabel(LABELS[metric])
    ax.set_title(f'{dimension.upper()} | {ratio}% updates | {LABELS[metric]}')
    ax.legend()
    ax.grid(axis='y', alpha=0.2)
    ax.set_axisbelow(True)
    fig.tight_layout()
    filename = GRAPHS / f'{dimension}_{metric}_updates_{ratio}.png'
    fig.savefig(filename, dpi=180)
    plt.close(fig)

def write_speedups(summary, dimension):
    by_config = defaultdict(dict)
    for r in summary:
        by_config[(r['size'], r['operations'], r['update_percent'])][r['algorithm']] = r
    rows = []
    f_name, s_name = ('Fenwick', 'Segment') if dimension == '1d' else ('Fenwick2D', 'Segment2D')
    for (size, ops, ratio), data in sorted(by_config.items()):
        f, s = data[f_name], data[s_name]
        record = dict(dimension=dimension, size=size, operations=ops, update_percent=ratio)
        for metric in METRICS:
            fv, sv = f[f'{metric}_mean'], s[f'{metric}_mean']
            record[f'{metric}_fenwick'] = fv
            record[f'{metric}_segment'] = sv
            # >1 means Fenwick wins for latency/time; throughput ratio is reversed.
            record[f'{metric}_fenwick_advantage_x'] = (
                (fv / sv if metric == 'throughput_ops_sec' else sv / fv) if fv and sv else math.nan
            )
        rows.append(record)
    path = PROCESSED / f'benchmark_{dimension}_speedups.csv'
    with path.open('w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    return path

def main():
    PROCESSED.mkdir(parents=True, exist_ok=True)
    GRAPHS.mkdir(parents=True, exist_ok=True)
    for dimension in ('1d', '2d'):
        raw = read_csv(dimension)
        summary = summarize(raw, dimension)
        write_summary(summary, dimension)
        write_speedups(summary, dimension)
        for ratio in sorted({r['update_percent'] for r in raw}):
            for metric in METRICS:
                plot_metric(summary, dimension, metric, ratio)
        print(f'{dimension.upper()}: {len(raw)} raw records, {len(summary)} grouped results')
    print(f'Summaries saved in {PROCESSED}; 30 graphs saved in {GRAPHS}')

if __name__ == '__main__':
    main()
