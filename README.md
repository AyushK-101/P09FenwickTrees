# Fenwick trees versus segment trees (1D and 2D)

**CS G526 — Advanced Algorithms and Complexity | P09**

This project implements and evaluates Fenwick Trees (Binary Indexed Trees) and Segment Trees for dynamic sum queries in one and two dimensions. The comparison uses the same generated workloads for both structures and measures construction time, update latency, query latency, total execution time, throughput, and whole-process CPU/cache/memory behavior.

The full technical report is available as `P09_Fenwick_Segment_Report_Green.pdf` (or as `report.pdf` inside the LaTeX project). It contains the explanations, implementation listings, experiment methodology, measured results, and limitations.

## Implemented functionality

| Implementation | Supported operations | Notes |
|---|---|---|
| 1D Fenwick Tree | Point addition, prefix sum, range sum | Uses `i & -i` to navigate the array |
| 1D Fenwick variants | Update/query demonstrations, linear construction, order statistics | Order-statistic search assumes nonnegative frequencies |
| 2D Fenwick Tree | Point addition, 2D prefix sum, rectangle sum | Nested low-bit traversals |
| 1D Segment Tree | Point addition, range sum | Array-based implementation |
| 2D Segment Tree | Point addition, rectangle sum | Array-based implementation |

Both Segment Tree implementations are sum-query baselines. **Lazy propagation, pointer-based nodes, and range-minimum/range-maximum implementations are not part of the tested code.**

## Repository layout

```text
P09FenwickTrees/
├── src/
│   ├── fenwick/
│   │   ├── fenwick_1d.cpp
│   │   ├── fenwick_1d_update.cpp
│   │   ├── fenwick_1d_query.cpp
│   │   ├── fenwick_1d_linear_build.cpp
│   │   ├── fenwick_2d.cpp
│   │   └── fenwick_order_statistics.cpp
│   └── segment_tree/
│       ├── segment_1d.cpp
│       └── segment_2d.cpp
├── tests/
│   ├── test_correctness.cpp
│   └── test_fenwick_advanced.cpp
├── benchmarks/
│   ├── benchmark_1d.cpp
│   └── benchmark_2d.cpp
├── profiling/
│   ├── profile_1d.cpp
│   └── profile_2d.cpp
├── scripts/
│   └── analyze_results.py
├── results/
│   ├── raw/
│   ├── processed/
│   └── graphs/
├── docs/
├── report/
└── README.md
```

`build/`, `results/`, `scripts/`, `docs/`, and `report/` may need to be created or populated if starting from the source-only ZIP. The report's editable LaTeX package is distributed separately.

## Requirements

- Linux or Ubuntu on WSL; a C++17 compiler such as `g++`
- Python 3 and Matplotlib for benchmark analysis and graphs
- Optional: Linux `perf`, Valgrind Cachegrind, and `/usr/bin/time` for profiling
- Optional: XeLaTeX and the fonts used by the report for rebuilding the PDF

Run all commands from the project root:

```bash
cd ~/projects/P09FenwickTrees
mkdir -p build results/raw results/processed results/graphs scripts
```

## Build and run the correctness tests

```bash
g++ -std=c++17 -O2 tests/test_correctness.cpp -o build/test_correctness
g++ -std=c++17 -O2 tests/test_fenwick_advanced.cpp -o build/test_fenwick_advanced

./build/test_correctness
./build/test_fenwick_advanced
```

The correctness suite checks the main 1D/2D data structures against straightforward reference calculations. The advanced suite exercises the linear Fenwick build and order-statistic functionality. A passing test run verifies the cases covered by those tests, not every possible input.

## Reproduce the benchmarks

```bash
g++ -std=c++17 -O2 benchmarks/benchmark_1d.cpp -o build/benchmark_1d
g++ -std=c++17 -O2 benchmarks/benchmark_2d.cpp -o build/benchmark_2d

./build/benchmark_1d
./build/benchmark_2d
```

The programs create these files automatically:

- `results/raw/benchmark_1d.csv`
- `results/raw/benchmark_2d.csv`

| Benchmark | Sizes | Operations per run | Update proportions | Trials per configuration |
|---|---|---:|---|---:|
| 1D | 10,000; 100,000; 1,000,000 | 100,000 | 10%, 50%, 90% | 5 |
| 2D | 50×50; 100×100; 250×250 | 10,000 | 10%, 50%, 90% | 5 |

The code uses batches of 100 operations for timing and alternates the execution order of the two implementations between trials. The update percentages refer to point additions; the remaining operations are range-sum or rectangle-sum queries. Results are dependent on the hardware, compiler, system load, and tested implementation.

The raw CSVs contain `build_ms`, `update_ns`, `query_ns`, `total_ms`, `throughput_ops_sec`, and a query-result checksum. Corresponding checksums are a useful cross-check, but the separate correctness tests provide stronger validation.

## Summarize measurements and generate graphs

Copy `analyze_results.py` into `scripts/` if it is not already present. Install Matplotlib in a Python virtual environment:

```bash
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install matplotlib
python3 scripts/analyze_results.py
```

The script reads the two raw benchmark CSVs and produces:

```text
results/processed/benchmark_1d_summary.csv
results/processed/benchmark_2d_summary.csv
results/processed/benchmark_1d_speedups.csv
results/processed/benchmark_2d_speedups.csv
results/graphs/*.png
```

The summaries report averages and sample standard deviations across trials. The speedup files compare Fenwick and Segment Tree measurements for the same configuration. The analysis script generates 30 graphs (five metrics × three update proportions × two dimensions). The final report selects a smaller set of figures and uses a green editorial palette; the analysis script's default plot styling may differ.

To open the graph folder from WSL:

```bash
explorer.exe "$(wslpath -w "$PWD/results/graphs")"
```

## Reproduce CPU, cache, and memory profiling

```bash
g++ -std=c++17 -O2 profiling/profile_1d.cpp -o build/profile_1d
g++ -std=c++17 -O2 profiling/profile_2d.cpp -o build/profile_2d
```

The profiling executables accept the algorithm name and workload parameters:

```bash
./build/profile_1d fenwick 100000 50 100000
./build/profile_1d segment 100000 50 100000

./build/profile_2d fenwick 250 250 50 100000
./build/profile_2d segment 250 250 50 100000
```

Example Linux profiling commands:

```bash
perf stat -r 5 -e cycles,instructions,cache-references,cache-misses \
  ./build/profile_2d fenwick 250 250 50 100000

valgrind --tool=cachegrind --cache-sim=yes \
  ./build/profile_2d fenwick 250 250 50 100000

/usr/bin/time -v ./build/profile_2d fenwick 250 250 50 100000
```

Repeat for `segment` and for the chosen 1D workload. `perf` may require system permissions. Hardware counter values and Cachegrind's **simulated** cache counts are different measurement types. Profiling covers the entire process, including initialization and output; maximum resident set size is process-level peak memory rather than exact tree allocation.

## Main findings

Across the nine tested configurations in each dimension, Fenwick Trees had lower measured point-update latency, range-query latency, and total benchmark execution time than the corresponding Segment Trees. The reported configuration-level total-time speedups were approximately **1.82–4.17× in 1D** and **1.60–3.35× in 2D**.

Segment Trees generally had lower measured construction times in these experiments. The benchmarked Fenwick constructors use repeated point updates; the separately implemented linear-time 1D Fenwick construction was not substituted into the main benchmark. The observed advantage therefore concerns the tested implementations and workloads, not every possible construction strategy or supported query operation.
