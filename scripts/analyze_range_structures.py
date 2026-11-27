#!/usr/bin/env python3
"""Summarize a completed range-structure run without touching the original results."""

import argparse
import csv
from collections import defaultdict
from math import isfinite
from pathlib import Path
from statistics import median, quantiles

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter


TITLES = {
    "construction": "Construction from an identical array",
    "experiment_a": "Point addition and range sum",
    "experiment_b": "Range addition and range sum",
    "experiment_c": "Range assignment and range sum",
}
ALGORITHMS = {
    "construction": ("Fenwick (linear)", "Segment (iterative)", "Fenwick (repeated)"),
    "experiment_a": ("Fenwick (linear)", "Segment (iterative)"),
    "experiment_b": ("Fenwick (two BITs)", "Segment (lazy)"),
    "experiment_c": ("Segment (lazy)",),
}
METRICS = {
    "build_ms": "Construction (ms)",
    "update_ns": "Mean update latency (ns)",
    "query_ns": "Mean query latency (ns)",
    "total_ms": "Mixed workload (ms)",
    "throughput_ops_sec": "Throughput (M ops/s)",
    "storage_bytes": "Tree storage (MiB)",
    "clock_pair_ns": "Empty timer interval (ns)",
}
SIZES = (10_000, 100_000, 1_000_000)
RATIOS = (10, 50, 90)
TRIALS = set(range(1, 7))


def read_trials(directory, experiment):
    path = directory / f"{experiment}.csv"
    construction = experiment == "construction"
    metrics = ("build_ms", "storage_bytes") if construction else tuple(METRICS)
    spans = ("point",) if experiment == "experiment_a" else ("short", "wide")
    expected = {(n, "", 0) for n in SIZES} if construction else {
        (n, span, ratio) for n in SIZES for span in spans for ratio in RATIOS
    }
    groups = defaultdict(dict)
    with path.open(newline="", encoding="utf-8") as source:
        for row in csv.DictReader(source):
            n = int(row["n"])
            key = (n, "", 0) if construction else (
                n, row["update_span"], int(row["update_percent"])
            )
            trial = int(row["trial"])
            algorithm = row["algorithm"]
            sample = (algorithm, trial)
            if sample in groups[key]:
                raise ValueError(f"{path}: duplicate row for {key}, {sample}")
            if int(row["input_seed"]) != 42 or (
                not construction and (int(row["operations"]) != 100_000 or
                                      int(row["workload_seed"]) != 12345 + 100 * trial)
            ):
                raise ValueError(f"{path}: unexpected workload definition for {key}, {sample}")
            measurements = {metric: float(row[metric]) for metric in metrics}
            if any(not isfinite(value) or value <= 0 for value in measurements.values()):
                raise ValueError(f"{path}: nonpositive or nonfinite measurement for {key}, {sample}")
            groups[key][sample] = {**measurements, "checksum": int(row["checksum"])}

    if set(groups) != expected:
        raise ValueError(f"{path}: missing or unexpected configurations")
    samples = {(name, trial) for name in ALGORITHMS[experiment] for trial in TRIALS}
    for key, group in groups.items():
        if set(group) != samples:
            raise ValueError(f"{path}: incomplete or unexpected trials for {key}")
        for trial in TRIALS:
            if len({group[name, trial]["checksum"] for name in ALGORITHMS[experiment]}) != 1:
                raise ValueError(f"{path}: mismatched checksums for {key}, trial {trial}")
        if construction and len({row["checksum"] for row in group.values()}) != 1:
            raise ValueError(f"{path}: construction changed between trials for {key}")
    return groups, metrics


def describe(values):
    q1, _, q3 = quantiles(values, n=4, method="inclusive")
    return {"median": median(values), "q1": q1, "q3": q3}


def summarize(groups, metrics, experiment):
    rows = []
    paired = []
    baseline, *comparators = ALGORITHMS[experiment]
    for (n, span, ratio), samples in sorted(groups.items()):
        config = dict(n=n, update_span=span, update_percent=ratio)
        for name in ALGORITHMS[experiment]:
            row = dict(**config, algorithm=name, trials=len(TRIALS))
            for metric in metrics:
                stats = describe([samples[name, trial][metric] for trial in sorted(TRIALS)])
                row.update({f"{metric}_{stat}": value for stat, value in stats.items()})
            rows.append(row)
        for name in comparators:
            for metric in metrics:
                if metric == "clock_pair_ns":
                    continue
                ratios = [samples[name, trial][metric] / samples[baseline, trial][metric]
                          for trial in sorted(TRIALS)]
                paired.append(dict(**config, baseline=baseline, comparator=name,
                                   metric=metric, **describe(ratios)))
    return rows, paired


def write_csv(path, rows):
    if rows:
        with path.open("x", newline="", encoding="utf-8") as output:
            writer = csv.DictWriter(output, fieldnames=list(rows[0]))
            writer.writeheader()
            writer.writerows(rows)


def plot(directory, experiment, rows, metrics):
    colors = ("#8C2D19", "#6E6A63", "#A8A298")
    markers = ("o", "s", "^")
    divisors = {"storage_bytes": 1024 ** 2, "throughput_ops_sec": 1e6}
    metrics = [metric for metric in metrics if metric != "clock_pair_ns"]
    construction = experiment == "construction"
    for span, ratio in sorted({(row["update_span"], row["update_percent"]) for row in rows}):
        selected = {(row["algorithm"], row["n"]): row for row in rows
                    if row["update_span"] == span and row["update_percent"] == ratio}
        if construction:
            fig, axes = plt.subplots(1, 2, figsize=(9.4, 4.2))
            title = TITLES[experiment]
        else:
            fig, axes = plt.subplots(2, 3, figsize=(12.2, 7.1))
            title = f"{TITLES[experiment]} · {ratio}% updates · {span} intervals"
        fig.suptitle(title, fontfamily="serif", fontsize=19, y=0.97)
        handles = []
        for ax, metric in zip(axes.flat, metrics):
            divisor = divisors.get(metric, 1)
            ax.set_title(METRICS[metric], loc="left", fontsize=11)
            for i, name in enumerate(ALGORITHMS[experiment]):
                group = [selected[name, n] for n in SIZES]
                medians = [row[f"{metric}_median"] / divisor for row in group]
                lower = [value - row[f"{metric}_q1"] / divisor for row, value in zip(group, medians)]
                upper = [row[f"{metric}_q3"] / divisor - value for row, value in zip(group, medians)]
                handle = ax.errorbar(
                    range(len(SIZES)), medians, yerr=(lower, upper), label=name,
                    marker=markers[i], color=colors[i], linestyle=("-", "--", ":")[i],
                    linewidth=1.45, markersize=5, capsize=3,
                )
                if ax is axes.flat[0]:
                    handles.append(handle)
            ax.set_xticks(range(len(SIZES)), ("10k", "100k", "1M"))
            ax.yaxis.set_major_formatter(FuncFormatter(lambda value, _: f"{value:,.3g}"))
            ax.set_ylim(bottom=0)
            ax.grid(axis="y", color="#DED9CF", linewidth=0.65)
            ax.set_axisbelow(True)
            ax.spines[["top", "right", "left"]].set_visible(False)
            ax.spines["bottom"].set_color("#DED9CF")
            ax.tick_params(length=0, pad=6)
        fig.legend(handles=handles, ncol=len(handles), loc="upper center",
                   bbox_to_anchor=(0.5, 0.90), frameon=False)
        fig.text(0.5, 0.035, "Array size in elements  ·  Median and interquartile range across six trials",
                 ha="center", color="#6E6A63", fontsize=9)
        fig.tight_layout(rect=(0.025, 0.075, 0.99, 0.82), h_pad=2.2, w_pad=2.1)
        filename = experiment if construction else f"{experiment}_{span}_{ratio:02d}"
        fig.savefig(directory / f"{filename}.png", dpi=145)
        plt.close(fig)


def write_findings(path, results):
    indexed = {
        experiment: {(row["n"], row["update_span"], row["update_percent"], row["algorithm"]): row
                     for row in summary}
        for experiment, (summary, _, _) in results.items()
    }
    n = SIZES[-1]
    lines = ["# Construction and range-update results", "",
             "The tables report medians across six trials.", "",
             f"## Construction at {n:,} elements", "",
             "| Builder | Build (ms) | Tree storage (MiB) |",
             "|---|---:|---:|"]
    for name in ALGORITHMS["construction"]:
        row = indexed["construction"][n, "", 0, name]
        lines.append(f'| {name} | {row["build_ms_median"]:.3f} | '
                     f'{row["storage_bytes_median"] / 1024 ** 2:.2f} |')
    lines.extend(["",
                  "The two Fenwick builders use the same representation. Their build-time difference "
                  "measures the change in construction procedure, including its initialization work.",
                  "", f"## Workload timings at {n:,} elements and 50% updates", "",
                  "| Update | Span | Tree | Update (ns) | Query (ns) | Replay (ms) |",
                  "|---|---|---|---:|---:|---:|"])
    for experiment, operation, spans in (
        ("experiment_a", "Addition", ("point",)),
        ("experiment_b", "Addition", ("short", "wide")),
        ("experiment_c", "Assignment", ("short", "wide")),
    ):
        for span in spans:
            for name in ALGORITHMS[experiment]:
                row = indexed[experiment][n, span, 50, name]
                lines.append(f'| {operation} | {span} | {name} | {row["update_ns_median"]:.1f} | '
                             f'{row["query_ns_median"]:.1f} | {row["total_ms_median"]:.3f} |')

    lines.extend(["", "## Replay time across all paired configurations", "",
                  "Configurations are classified by the median of the six paired "
                  "comparator/baseline duration ratios.", "",
                  "| Workload | Fenwick lower | Segment lower | Equal |",
                  "|---|---:|---:|---:|"])
    for experiment, span, operation in (("experiment_a", "point", "Point addition"),
                                        ("experiment_b", "short", "Short range addition"),
                                        ("experiment_b", "wide", "Wide range addition")):
        ratios = [row["median"] for row in results[experiment][1]
                  if row["metric"] == "total_ms" and row["update_span"] == span]
        lines.append(f"| {operation} | {sum(ratio > 1 for ratio in ratios)} | "
                     f"{sum(ratio < 1 for ratio in ratios)} | {ratios.count(1)} |")
    lines.extend(["",
                  "Replay timing excludes construction. Range-addition timings reflect both the "
                  "boundary updates in the two-BIT tree and the recursive traversal and tag pushes "
                  "in the lazy tree. Partial queries can trigger those pushes. The experiment "
                  "compares these implementations under the sampled interval distributions.",
                  "",
                  "Assignment timings show how the lazy implementation behaves as the interval "
                  "distribution and update proportion vary. A Fenwick point-addition tree would "
                  "need to derive a separate delta for each overwritten value.",
                  "",
                  "Operation latency includes the clock measurement. Use the empty timer "
                  "interval reported in the summaries when interpreting small differences.",
                  ""])
    with path.open("x", encoding="utf-8") as output:
        output.write("\n".join(lines))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run_dir", type=Path, help="Directory containing all four raw CSVs")
    args = parser.parse_args()
    results = {}
    for experiment in TITLES:
        groups, metrics = read_trials(args.run_dir, experiment)
        summary, paired = summarize(groups, metrics, experiment)
        results[experiment] = (summary, paired, metrics)

    output = args.run_dir / "analysis"
    graphs = output / "graphs"
    graphs.mkdir(parents=True, exist_ok=False)
    with plt.rc_context({
        "figure.facecolor": "#FBF9F5", "axes.facecolor": "#FBF9F5",
        "axes.titlecolor": "#1F1E1C", "text.color": "#1F1E1C",
        "xtick.color": "#6E6A63", "ytick.color": "#6E6A63",
        "font.family": ["DejaVu Sans"],
    }):
        for experiment, (summary, paired, metrics) in results.items():
            write_csv(output / f"{experiment}_summary.csv", summary)
            write_csv(output / f"{experiment}_paired_ratios.csv", paired)
            plot(graphs, experiment, summary, metrics)
    write_findings(output / "findings.md", results)
    print(f"Validated summaries and figures saved to {output}")


if __name__ == "__main__":
    main()
