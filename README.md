# Fenwick trees versus segment trees (1D and 2D)

**CS G526 — Advanced Algorithms and Complexity | P09**

**Video:** https://www.youtube.com/watch?v=FM50SLNacqw  
**Presentation:** [`ppt/`](ppt/)  
**Technical report:** [`report/`](report/)  
**Proposal:** [`proposal/`](proposal/)

This project implements and experimentally compares Fenwick Trees (Binary Indexed Trees) and Segment Trees for dynamic sum queries in one and two dimensions.

Both structures support efficient point updates and range-sum queries, but differ in their internal organization, traversal patterns, construction methods, and memory access behavior.

The project investigates these differences through correctness testing, controlled benchmarks, and CPU/cache/memory profiling.

The experiments measure:

- Construction time
- Point-update latency
- Range-query latency
- Total execution time
- Throughput
- CPU instructions and cycles
- Cache references and misses
- Memory consumption

The full technical report is available in `report/`.

## 1. Implemented functionality

| Implementation | Supported operations | Main technique |
|---|---|---|
| 1D Fenwick Tree | Point addition, prefix sum, range sum | Low-bit index traversal |
| 1D Fenwick variants | Linear construction, order statistics, update/query demonstrations | Bit manipulation and bit lifting |
| 2D Fenwick Tree | Point addition, 2D prefix sum, rectangle sum | Nested low-bit traversal |
| 1D Segment Tree | Point addition, range sum | Recursive interval decomposition |
| 2D Segment Tree | Point addition, rectangle sum | Recursive row and column decomposition |

All implementations use C++17.

The Fenwick implementations use array-based indexing without explicit tree pointers. The Segment Tree implementations also use array-based storage.

The tested Segment Trees support sum queries and point additions. Lazy propagation, pointer-based nodes, and range-minimum/range-maximum implementations are not included.

## 2. Repository structure

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
│   │   ├── benchmark_1d.csv
│   │   └── benchmark_2d.csv
│   ├── processed/
│   │   ├── benchmark_1d_summary.csv
│   │   ├── benchmark_2d_summary.csv
│   │   ├── benchmark_1d_speedups.csv
│   │   └── benchmark_2d_speedups.csv
│   └── graphs/
├── profiling/
├── docs/
├── ppt/
├── proposal/
├── report/
└── README.md
```

The `results/graphs/` directory contains the generated performance-comparison figures.

## 3. Requirements

The project was developed and tested using Ubuntu on WSL.

Required software:

- C++17-compatible compiler (`g++`)
- Python 3
- Matplotlib for statistical graphs

Additional profiling tools:

- Linux `perf`
- Valgrind Cachegrind
- GNU `/usr/bin/time`

Run commands from the project root:

```bash
cd ~/projects/P09FenwickTrees
mkdir -p build results/raw results/processed results/graphs
```

## 4. Correctness testing

Before comparing performance, both data structures must produce the correct results.

The correctness tests compare tree outputs against straightforward reference calculations.

Compile:

```bash
g++ -std=c++17 -O2 tests/test_correctness.cpp -o build/test_correctness
g++ -std=c++17 -O2 tests/test_fenwick_advanced.cpp -o build/test_fenwick_advanced
```

Run:

```bash
./build/test_correctness
./build/test_fenwick_advanced
```

`test_correctness.cpp` tests the main 1D and 2D Fenwick and Segment Tree implementations.

`test_fenwick_advanced.cpp` tests linear Fenwick construction and order-statistic functionality.

Randomized tests exercise repeated updates and queries. Passing these tests establishes correctness for the tested cases; it does not constitute a formal proof for every possible input.

## 5. Benchmark methodology

The benchmark programs generate workloads and execute corresponding operations on both data structures.

The purpose is to compare the performance of the implementations under matching experimental conditions.

### 5.1 Experimental variables

| Variable | 1D experiments | 2D experiments |
|---|---|---|
| Data structure | Fenwick vs Segment Tree | Fenwick2D vs Segment2D |
| Input sizes | 10,000; 100,000; 1,000,000 | 50×50; 100×100; 250×250 |
| Operations per run | 100,000 | 10,000 |
| Update proportions | 10%, 50%, 90% | 10%, 50%, 90% |
| Trials per configuration | 5 | 5 |
| Timing batch size | 100 operations | 100 operations |

The update proportions produce three workload categories.

| Workload | Point updates | Queries | Interpretation |
|---|---:|---:|---|
| Read-heavy | 10% | 90% | Most operations request sums |
| Balanced | 50% | 50% | Equal proportions of updates and queries |
| Write-heavy | 90% | 10% | Most operations modify values |

For example, a balanced 1D workload contains 50,000 point additions and 50,000 range-sum queries out of 100,000 operations.

The benchmark programs use batches of 100 operations and alternate the execution order of Fenwick and Segment Trees between trials.

Both structures receive equivalent generated workloads. Measurements are repeated five times to reduce dependence on a single run.

### 5.2 Performance metrics

The experiments measure five primary metrics.

| Metric | CSV field | Unit | Meaning | Preferred direction |
|---|---|---|---|---|
| Construction time | `build_ms` | Milliseconds | Time required to construct the data structure | Lower |
| Update latency | `update_ns` | Nanoseconds | Average measured time per point update | Lower |
| Query latency | `query_ns` | Nanoseconds | Average measured time per range query | Lower |
| Total execution time | `total_ms` | Milliseconds | Time spent executing the benchmark workload | Lower |
| Throughput | `throughput_ops_sec` | Operations/second | Number of operations processed per second | Higher |

**Construction time** measures how quickly an implementation prepares its internal data structure from the input.

**Update latency** measures the time needed to add a value at a specified position. In 2D, this means modifying a particular matrix cell.

**Query latency** measures the time needed to calculate a requested sum. In 1D, the query covers an interval; in 2D, it covers a rectangle.

**Total execution time** measures the elapsed time for the benchmark's operation workload. It is not the same as the construction-time metric.

**Throughput** expresses how many operations can be completed per second. A higher throughput indicates that more work is processed in the same amount of time.

### 5.3 Benchmark execution

Compile:

```bash
g++ -std=c++17 -O2 benchmarks/benchmark_1d.cpp -o build/benchmark_1d
g++ -std=c++17 -O2 benchmarks/benchmark_2d.cpp -o build/benchmark_2d
```

Execute:

```bash
./build/benchmark_1d
./build/benchmark_2d
```

The generated measurements are written to:

```text
results/raw/benchmark_1d.csv
results/raw/benchmark_2d.csv
```

## 6. Raw benchmark CSV files

### 6.1 `benchmark_1d.csv` — Individual 1D measurements

**Location:** `results/raw/benchmark_1d.csv`

This file contains the measurements from every 1D benchmark trial.

Each row represents one algorithm, one array size, one update proportion, and one trial.

| Column | Description |
|---|---|
| `algorithm` | `Fenwick` or `Segment` |
| `n` | Number of elements in the input array |
| `operations` | Total operations in the workload |
| `update_percent` | Percentage of operations that are point updates |
| `trial` | Trial number, from 1 to 5 |
| `build_ms` | Construction time in milliseconds |
| `update_ns` | Average measured point-update latency |
| `query_ns` | Average measured range-query latency |
| `total_ms` | Total benchmark operation time |
| `throughput_ops_sec` | Operations processed per second |
| `checksum` | Aggregate of query results used for comparison |

There are 90 measurement rows:

```text
3 input sizes
× 3 update proportions
× 5 trials
× 2 algorithms
= 90 measurements
```

The checksum helps verify that both implementations produce matching aggregate query results under corresponding workloads. Matching checksums alone do not prove that every individual query result matches.

### 6.2 `benchmark_2d.csv` — Individual 2D measurements

**Location:** `results/raw/benchmark_2d.csv`

This file follows the same structure as the 1D benchmark but uses matrix dimensions.

Its columns are:

```text
algorithm
rows
cols
operations
update_percent
trial
build_ms
update_ns
query_ns
total_ms
throughput_ops_sec
checksum
```

The `rows` and `cols` columns specify the matrix dimensions.

For example, `rows = 250` and `cols = 250` represent a matrix containing 62,500 cells.

This file also contains 90 measurement rows.

## 7. Statistical analysis and processed CSVs

The script `scripts/analyze_results.py` processes the raw measurements, computes statistical summaries, compares algorithms, and generates graphs.

Run:

```bash
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install matplotlib
python3 scripts/analyze_results.py
```

The script generates four processed CSV files and 30 comparison graphs.

### 7.1 `benchmark_1d_summary.csv` — Mean and variation in 1D performance

**Location:** `results/processed/benchmark_1d_summary.csv`

This file combines the five trials of each configuration.

Instead of examining five separate timing measurements, the reader can examine their mean and sample standard deviation.

The file contains:

- `dimension`: `1d`
- `size`: Array length
- `operations`: Number of operations
- `update_percent`: Update percentage
- `algorithm`: Fenwick or Segment
- `trials`: Number of trials
- `build_ms_mean`, `build_ms_std`
- `update_ns_mean`, `update_ns_std`
- `query_ns_mean`, `query_ns_std`
- `total_ms_mean`, `total_ms_std`
- `throughput_ops_sec_mean`, `throughput_ops_sec_std`

The suffix `_mean` indicates the arithmetic average across trials.

The suffix `_std` indicates the sample standard deviation, which describes how much the trial results vary around their mean.

For example, a large standard deviation relative to the mean indicates that the observed execution time was less stable across trials.

The file contains 18 grouped results: nine configurations for each of two algorithms.

### 7.2 `benchmark_2d_summary.csv` — Mean and variation in 2D performance

**Location:** `results/processed/benchmark_2d_summary.csv`

This file performs the same statistical analysis for the 2D experiments.

The `size` field identifies matrix dimensions such as `50x50`, `100x100`, and `250x250`.

It contains 18 grouped results.

### 7.3 `benchmark_1d_speedups.csv` — Direct 1D speedup comparisons

**Location:** `results/processed/benchmark_1d_speedups.csv`

This file places the average Fenwick and Segment Tree measurements side by side.

For each metric, it contains the Fenwick value, the Segment Tree value, and a calculated Fenwick advantage ratio.

Examples:

```text
build_ms_fenwick
build_ms_segment
build_ms_fenwick_advantage_x

update_ns_fenwick
update_ns_segment
update_ns_fenwick_advantage_x

query_ns_fenwick
query_ns_segment
query_ns_fenwick_advantage_x

total_ms_fenwick
total_ms_segment
total_ms_fenwick_advantage_x

throughput_ops_sec_fenwick
throughput_ops_sec_segment
throughput_ops_sec_fenwick_advantage_x
```

For latency and execution-time metrics:

```text
Fenwick advantage = Segment time / Fenwick time
```

For throughput:

```text
Fenwick advantage = Fenwick throughput / Segment throughput
```

Interpretation:

- Greater than 1×: Fenwick performs better on that metric.
- Equal to 1×: Both have equal measured performance.
- Less than 1×: Segment Tree performs better.

For example, a 3× update-latency advantage means the Segment Tree's average update time is three times the Fenwick Tree's average update time.

The construction-time ratio uses the same direction convention, so a value below 1× indicates faster Segment Tree construction.

### 7.4 `benchmark_2d_speedups.csv` — Direct 2D speedup comparisons

**Location:** `results/processed/benchmark_2d_speedups.csv`

This file applies the same speedup calculations to the 2D experiments.

It allows comparisons across different matrix dimensions and update proportions.

## 8. Experiment 1 — Construction time comparison

**Research question:** Which data structure takes less time to construct from the initial input?

The first experiment compares the construction cost of Fenwick Trees and Segment Trees.

Construction happens before the measured sequence of updates and queries.

### Method

The same input size is used for both algorithms.

Construction time is recorded in milliseconds and repeated across five trials.

The experiment is performed separately for 1D arrays and 2D matrices.

### Graphs

| Graph | Description |
|---|---|
| [`1d_build_ms_updates_10.png`](results/graphs/1d_build_ms_updates_10.png) | 1D construction times under the 10% update configuration |
| [`1d_build_ms_updates_50.png`](results/graphs/1d_build_ms_updates_50.png) | 1D construction times under the balanced configuration |
| [`1d_build_ms_updates_90.png`](results/graphs/1d_build_ms_updates_90.png) | 1D construction times under the 90% update configuration |
| [`2d_build_ms_updates_10.png`](results/graphs/2d_build_ms_updates_10.png) | 2D construction times under the 10% update configuration |
| [`2d_build_ms_updates_50.png`](results/graphs/2d_build_ms_updates_50.png) | 2D construction times under the balanced configuration |
| [`2d_build_ms_updates_90.png`](results/graphs/2d_build_ms_updates_90.png) | 2D construction times under the 90% update configuration |

### Findings

Segment Trees generally showed lower construction times in the tested configurations.

The benchmarked Fenwick constructors use repeated point updates, while the Segment Tree constructors recursively build their internal representation.

The separate linear-time 1D Fenwick constructor was implemented and tested for correctness but was not substituted into the main benchmark.

Therefore, these graphs compare the construction strategies actually used by the benchmark programs, not every possible Fenwick construction method.

## 9. Experiment 2 — Point-update latency comparison

**Research question:** Which data structure modifies a value faster?

A point update adds a value to a specified position.

In 1D:

```text
A[i] += delta
```

In 2D:

```text
A[row][col] += delta
```

The data structure must update its internal representation so that subsequent sum queries remain correct.

### Method

The benchmark measures point-update latency in nanoseconds.

The update positions and delta values come from the generated workloads.

Both structures process corresponding updates under the same configuration.

### Graphs

| Graph | Description |
|---|---|
| [`1d_update_ns_updates_10.png`](results/graphs/1d_update_ns_updates_10.png) | 1D update latency, read-heavy workload |
| [`1d_update_ns_updates_50.png`](results/graphs/1d_update_ns_updates_50.png) | 1D update latency, balanced workload |
| [`1d_update_ns_updates_90.png`](results/graphs/1d_update_ns_updates_90.png) | 1D update latency, write-heavy workload |
| [`2d_update_ns_updates_10.png`](results/graphs/2d_update_ns_updates_10.png) | 2D update latency, read-heavy workload |
| [`2d_update_ns_updates_50.png`](results/graphs/2d_update_ns_updates_50.png) | 2D update latency, balanced workload |
| [`2d_update_ns_updates_90.png`](results/graphs/2d_update_ns_updates_90.png) | 2D update latency, write-heavy workload |

### Findings

Fenwick Trees had lower measured point-update latency in all nine tested configurations in both dimensions.

The 1D update-latency advantage ranged from approximately 2.24× to 5.15×.

The 2D update-latency advantage ranged from approximately 1.39× to 3.42×.

The Fenwick implementations use compact low-bit loops, whereas the Segment Tree implementations use recursive interval traversal.

These differences are plausible contributors to the measured performance gap, although the benchmarks do not isolate each individual source of overhead.

## 10. Experiment 3 — Range-query latency comparison

**Research question:** Which data structure calculates sums faster?

In 1D, a range query calculates:

```text
sum(A[left ... right])
```

In 2D, a rectangle query calculates:

```text
sum(A[row1 ... row2][col1 ... col2])
```

Both structures return the same mathematical result, but use different traversal methods.

### Method

The benchmark measures average query latency in nanoseconds.

The 1D experiments use interval-sum queries.

The 2D experiments use rectangle-sum queries.

Each experiment is repeated across the specified input sizes and update proportions.

### Graphs

| Graph | Description |
|---|---|
| [`1d_query_ns_updates_10.png`](results/graphs/1d_query_ns_updates_10.png) | 1D query latency, read-heavy workload |
| [`1d_query_ns_updates_50.png`](results/graphs/1d_query_ns_updates_50.png) | 1D query latency, balanced workload |
| [`1d_query_ns_updates_90.png`](results/graphs/1d_query_ns_updates_90.png) | 1D query latency, write-heavy workload |
| [`2d_query_ns_updates_10.png`](results/graphs/2d_query_ns_updates_10.png) | 2D rectangle-query latency, read-heavy workload |
| [`2d_query_ns_updates_50.png`](results/graphs/2d_query_ns_updates_50.png) | 2D rectangle-query latency, balanced workload |
| [`2d_query_ns_updates_90.png`](results/graphs/2d_query_ns_updates_90.png) | 2D rectangle-query latency, write-heavy workload |

### Findings

Fenwick Trees had lower measured query latency in every tested configuration.

The 1D query-latency advantage ranged from approximately 1.57× to 4.40×.

The 2D query-latency advantage ranged from approximately 1.67× to 3.36×.

A Fenwick Tree calculates a range sum through prefix sums and subtraction.

A Segment Tree recursively identifies the nodes whose intervals cover the requested range.

For 2D queries, both approaches require additional work across two coordinates, but their memory access patterns and traversal overhead differ.

## 11. Experiment 4 — Total execution time comparison

**Research question:** Which implementation completes the same workload faster?

Individual operation latencies describe the cost of one type of operation.

Total execution time measures the overall cost of executing the benchmark workload, which contains both updates and queries.

This metric helps compare performance under realistic mixed-operation conditions.

### Method

Each implementation executes the generated workload for the same configuration.

The benchmark measures total elapsed execution time in milliseconds.

Three update-to-query ratios are evaluated.

### Graphs

| Graph | Description |
|---|---|
| [`1d_total_ms_updates_10.png`](results/graphs/1d_total_ms_updates_10.png) | 1D total execution time, read-heavy |
| [`1d_total_ms_updates_50.png`](results/graphs/1d_total_ms_updates_50.png) | 1D total execution time, balanced |
| [`1d_total_ms_updates_90.png`](results/graphs/1d_total_ms_updates_90.png) | 1D total execution time, write-heavy |
| [`2d_total_ms_updates_10.png`](results/graphs/2d_total_ms_updates_10.png) | 2D total execution time, read-heavy |
| [`2d_total_ms_updates_50.png`](results/graphs/2d_total_ms_updates_50.png) | 2D total execution time, balanced |
| [`2d_total_ms_updates_90.png`](results/graphs/2d_total_ms_updates_90.png) | 2D total execution time, write-heavy |

### Findings

Fenwick Trees completed the measured workloads faster in all tested configurations.

The observed total-time advantage ranged from:

| Dimension | Fenwick advantage |
|---|---:|
| 1D | Approximately 1.82–4.17× |
| 2D | Approximately 1.60–3.35× |

These results combine the effects of point updates and range queries.

The advantage varies with input size and operation mixture. It should not be interpreted as a fixed speedup for every workload.

## 12. Experiment 5 — Throughput comparison

**Research question:** How many operations can each implementation process per second?

Throughput measures the amount of work completed in a unit of time.

Unlike latency, where lower values are better, higher throughput indicates better performance.

### Method

Throughput is calculated from the benchmark's operation count and measured execution time.

The same array or matrix sizes and update proportions are used.

### Graphs

| Graph | Description |
|---|---|
| [`1d_throughput_ops_sec_updates_10.png`](results/graphs/1d_throughput_ops_sec_updates_10.png) | 1D throughput, read-heavy |
| [`1d_throughput_ops_sec_updates_50.png`](results/graphs/1d_throughput_ops_sec_updates_50.png) | 1D throughput, balanced |
| [`1d_throughput_ops_sec_updates_90.png`](results/graphs/1d_throughput_ops_sec_updates_90.png) | 1D throughput, write-heavy |
| [`2d_throughput_ops_sec_updates_10.png`](results/graphs/2d_throughput_ops_sec_updates_10.png) | 2D throughput, read-heavy |
| [`2d_throughput_ops_sec_updates_50.png`](results/graphs/2d_throughput_ops_sec_updates_50.png) | 2D throughput, balanced |
| [`2d_throughput_ops_sec_updates_90.png`](results/graphs/2d_throughput_ops_sec_updates_90.png) | 2D throughput, write-heavy |

### Findings

Fenwick Trees achieved higher throughput in the tested configurations.

This agrees with their lower measured total execution times.

Throughput and total execution time are related measurements rather than independent evidence of different performance properties.

## 13. Experiment 6 — Input-size scalability

**Research question:** How does the performance difference change when the data structure becomes larger?

A data structure that performs well for a small input may behave differently when the input grows.

Larger inputs can change memory access costs, cache behavior, and traversal overhead.

### Method

The benchmark repeats the same types of operations at increasing input sizes.

For 1D:

```text
10,000 elements
100,000 elements
1,000,000 elements
```

For 2D:

```text
50 × 50 cells
100 × 100 cells
250 × 250 cells
```

The speedup CSV files can be used to compare the relative performance of both algorithms as input size changes.

### Relevant results

```text
results/processed/benchmark_1d_speedups.csv
results/processed/benchmark_2d_speedups.csv
```

The total-time, update-latency, and query-latency graphs from Experiments 2–4 also show performance across input sizes.

### Findings

Fenwick Trees retained a measured advantage throughout the tested size ranges.

However, the size of that advantage changed between configurations.

In the 1D experiments, the relative advantage generally became smaller at one million elements than at some smaller input sizes.

This shows why performance comparisons should examine multiple input sizes rather than rely on one benchmark configuration.

## 14. Experiment 7 — CPU and instruction profiling

**Research question:** How much processor work does each implementation require?

Elapsed time alone does not explain why an implementation is faster.

CPU profiling provides additional information about the instructions executed and the processor cycles consumed.

### Tools

The project uses Linux `perf` for hardware performance counters.

Example:

```bash
perf stat -r 5 \
  -e cycles,instructions,cache-references,cache-misses \
  ./build/profile_2d fenwick 250 250 50 100000
```

Repeat the same command with `segment`.

### Metrics

| Metric | Meaning |
|---|---|
| CPU cycles | Number of processor clock cycles counted during execution |
| Instructions | Number of retired instructions |
| Instructions per cycle (IPC) | Average instructions retired per processor cycle |
| Cache references | Hardware-reported cache reference events |
| Cache misses | Hardware-reported cache miss events |

IPC is calculated as:

```text
IPC = Instructions / CPU cycles
```

A higher IPC means more instructions were retired per cycle, but does not automatically mean the program is faster.

### Measured 1D profiling results

| Metric | Fenwick | Segment Tree |
|---|---:|---:|
| CPU cycles | 36,871,900 | 47,425,949 |
| Instructions | 60,645,178 | 67,718,859 |
| IPC | 1.64 | 1.43 |
| Cache references | 2,542,967 | 1,917,881 |
| Cache misses | 177,298 | 156,483 |
| Cache miss rate | 6.97% | 8.16% |

Fenwick used approximately 22.3% fewer CPU cycles and 10.4% fewer instructions in this profiling comparison.

It had more absolute hardware cache misses but a lower miss rate. These two observations should not be confused.

### Measured 2D profiling results

| Metric | Fenwick2D | Segment2D |
|---|---:|---:|
| CPU cycles | 51,983,673 | 90,913,465 |
| Instructions | 106,227,450 | 116,912,825 |
| Cache references | 2,913,013 | 7,839,828 |
| Cache misses | 177,842 | 215,810 |

The 2D Fenwick implementation used fewer measured CPU cycles, instructions, cache references, and cache misses in this profiling configuration.

The corresponding `perf` wall-clock results were less consistent, so the CPU-cycle difference should not be presented as an identical elapsed-time speedup.

Profiling includes the complete process rather than only the tree operations.

## 15. Experiment 8 — Cache efficiency comparison

**Research question:** How do the implementations behave when accessing memory?

Modern processors use caches to reduce the cost of accessing main memory.

A cache miss occurs when requested data is not found at the relevant cache level and must be retrieved from another level of the memory hierarchy.

The project examines cache behavior using hardware counters and Cachegrind.

### Tools

```bash
valgrind --tool=cachegrind --cache-sim=yes \
  ./build/profile_2d fenwick 250 250 50 100000
```

Repeat with `segment`.

### Cachegrind metrics

| Metric | Meaning |
|---|---|
| `Ir` | Instructions executed |
| `Dr` | Data reads |
| `Dw` | Data writes |
| `D1mr` | Simulated L1 data-read misses |
| `D1mw` | Simulated L1 data-write misses |
| `DLmr` | Simulated last-level data-read misses |
| `DLmw` | Simulated last-level data-write misses |

### Measured 2D Cachegrind results

| Metric | Fenwick2D | Segment2D |
|---|---:|---:|
| Instructions (`Ir`) | 106,327,405 | 117,015,722 |
| Data reads (`Dr`) | 16,279,912 | 19,888,789 |
| L1 data-read misses (`D1mr`) | 2,145,020 | 4,320,000 |
| Data writes (`Dw`) | 7,208,043 | 7,522,554 |
| L1 data-write misses (`D1mw`) | 76,689 | 131,602 |

The Fenwick2D implementation produced approximately 50.3% fewer simulated L1 data-read misses in this configuration.

Cachegrind results are simulated cache measurements. They should not be treated as identical to the hardware cache events reported by `perf`.

The profiling programs also include initialization and workload-management activity.

## 16. Experiment 9 — Memory consumption comparison

**Research question:** How much process memory is used when running each implementation?

The project uses `/usr/bin/time -v` to record maximum resident set size.

Example:

```bash
/usr/bin/time -v \
  ./build/profile_2d fenwick 250 250 50 100000
```

Repeat with `segment`.

Maximum resident set size, or peak RSS, is the largest amount of physical memory occupied by the process during execution.

### Measured results

| Dimension | Fenwick | Segment Tree |
|---|---:|---:|
| 1D peak RSS | 7,688 KiB | 8,364 KiB |
| 2D peak RSS | 7,848 KiB | 9,356 KiB |

The measured Fenwick process used approximately 676 KiB less peak memory in the 1D profiling comparison and 1,508 KiB less in the 2D comparison.

These values represent whole-process memory, including program runtime and supporting allocations. They are not exact measurements of the tree storage alone.

## 17. How to read the performance graphs

All 30 benchmark graphs are stored in:

```text
results/graphs/
```

Their filenames follow this pattern:

```text
<dimension>_<metric>_updates_<percentage>.png
```

For example:

```text
1d_update_ns_updates_50.png
```

means:

- `1d`: One-dimensional experiment
- `update_ns`: Point-update latency in nanoseconds
- `updates_50`: Workload containing 50% updates

The graphs compare Fenwick and Segment Tree measurements at different input sizes.

### Graph interpretation

The horizontal axis identifies the input size.

The vertical axis identifies the measured performance metric.

Bars represent the average across five trials. Error bars represent sample standard deviation.

For construction time, update latency, query latency, and total execution time, shorter bars indicate better performance.

For throughput, taller bars indicate better performance.

The benchmark analysis graphs and the figures used in the final report may have different visual styling, but their underlying measurements come from the same benchmark data.

## 18. Graph index

The following table provides a complete index of the 30 generated benchmark figures.

| Comparison | 1D graphs | 2D graphs |
|---|---|---|
| Construction time | `1d_build_ms_updates_{10,50,90}.png` | `2d_build_ms_updates_{10,50,90}.png` |
| Point-update latency | `1d_update_ns_updates_{10,50,90}.png` | `2d_update_ns_updates_{10,50,90}.png` |
| Query latency | `1d_query_ns_updates_{10,50,90}.png` | `2d_query_ns_updates_{10,50,90}.png` |
| Total execution time | `1d_total_ms_updates_{10,50,90}.png` | `2d_total_ms_updates_{10,50,90}.png` |
| Throughput | `1d_throughput_ops_sec_updates_{10,50,90}.png` | `2d_throughput_ops_sec_updates_{10,50,90}.png` |

Each `{10,50,90}` represents three separate files.

For example:

```text
1d_build_ms_updates_10.png
1d_build_ms_updates_50.png
1d_build_ms_updates_90.png
```

Together, the five metrics, three update proportions, and two dimensions produce 30 graphs.

## 19. Reproducing the experiments

The following sequence reproduces the correctness tests, benchmarks, and statistical analysis.

### Step 1 — Compile the tests

```bash
g++ -std=c++17 -O2 tests/test_correctness.cpp -o build/test_correctness
g++ -std=c++17 -O2 tests/test_fenwick_advanced.cpp -o build/test_fenwick_advanced
```

### Step 2 — Run the tests

```bash
./build/test_correctness
./build/test_fenwick_advanced
```

### Step 3 — Compile the benchmarks

```bash
g++ -std=c++17 -O2 benchmarks/benchmark_1d.cpp -o build/benchmark_1d
g++ -std=c++17 -O2 benchmarks/benchmark_2d.cpp -o build/benchmark_2d
```

### Step 4 — Execute the benchmarks

```bash
./build/benchmark_1d
./build/benchmark_2d
```

### Step 5 — Generate the statistical summaries and graphs

```bash
source .venv/bin/activate
python3 scripts/analyze_results.py
```

### Step 6 — Inspect the outputs

```bash
ls results/raw/
ls results/processed/
ls results/graphs/
```

To open the graph directory in Windows:

```bash
explorer.exe "$(wslpath -w "$PWD/results/graphs")"
```

## 20. Main findings

The experiments show a consistent performance advantage for the tested Fenwick Tree implementations during dynamic sum operations.

Across the nine configurations in each dimension, Fenwick Trees achieved lower point-update latency, lower query latency, and lower total benchmark execution time than the corresponding Segment Trees.

The total execution-time advantage ranged from approximately 1.82× to 4.17× in 1D and 1.60× to 3.35× in 2D.

Segment Trees generally constructed faster in the main benchmarks, reflecting the construction methods used by the tested implementations.

The profiling measurements also showed differences in instruction counts, CPU cycles, simulated cache behavior, and peak process memory. These measurements help explain observed performance but do not isolate every source of execution overhead.

The results demonstrate the practical efficiency of low-bit indexing for dynamic sum queries. They do not imply that Fenwick Trees outperform Segment Trees for every operation, input distribution, implementation, or hardware platform.
