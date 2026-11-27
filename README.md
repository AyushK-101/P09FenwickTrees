# Fenwick trees versus segment trees (1D and 2D)

**Presentation:** [`ppt/`](ppt/)  
**Technical report:** [`report/`](report/)  
**Proposal:** [`proposal/`](proposal/)

This project implements and experimentally compares Fenwick Trees (Binary Indexed Trees) and Segment Trees for dynamic sum queries in one and two dimensions.

Both data structures support efficient point updates and range-sum queries, but differ in their internal representation, traversal methods, construction strategies, and memory access patterns.

The comparison uses matched generated workloads and evaluates construction time, point-update latency, query latency, total execution time, throughput, CPU instructions and cycles, cache behavior, and memory consumption.

The complete technical report, including implementation explanations, experimental methodology, measured results, and limitations, is available in `report/`.

## 1. Implemented functionality

| Implementation | Supported operations | Main technique |
|---|---|---|
| 1D Fenwick Tree | Point addition, prefix sum, range sum | Low-bit index traversal using `i & -i` |
| 1D Fenwick variants | Linear construction, order statistics, update/query demonstrations | Bit manipulation and bit lifting |
| 1D two-BIT Fenwick Tree | Range addition, prefix sum, range sum | Difference array and weighted prefix sums |
| 1D lazy Segment Tree | Range addition, range assignment, range sum | Deferred addition and assignment tags |
| 2D Fenwick Tree | Point addition, 2D prefix sum, rectangle sum | Nested low-bit traversal |
| 1D Segment Tree | Point addition, range sum | Iterative array traversal |
| 2D Segment Tree | Point addition, rectangle sum | Iterative row and column traversal |

All implementations use C++17.

The Fenwick implementations use array-based indexing without explicit tree pointers. The Segment Tree implementations also use array-based storage.

The standard 1D and 2D Segment Trees benchmark point additions and sum queries. The 1D lazy Segment Tree adds range addition and assignment. The 2D comparison covers point additions and rectangle sums.

The order-statistic Fenwick routine assumes nonnegative frequencies.

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
├── docs/
├── ppt/
├── proposal/
├── report/
└── README.md
```

The `results/graphs/` directory contains the generated benchmark figures.

The `build/` directory is created locally for compiled executables and is excluded from version control.

The extension adds `CMakeLists.txt`, a shared linear builder and two-BIT implementation under `src/fenwick/`, and the lazy implementation under `src/segment_tree/`. The new benchmark and test live in `benchmarks/benchmark_range_structures.cpp` and `tests/test_range_structures.cpp`. Windows tooling and analysis live under `scripts/`. The recorded extension data and graphs are in [results/range_windows_01](results/range_windows_01/).

### 2.1 Testing, benchmarking, and profiling files

The following programs verify correctness, generate performance measurements, and collect hardware-level profiling data.

| File | Purpose |
|---|---|
| `tests/test_correctness.cpp` | Checks 1D and 2D Fenwick Tree and Segment Tree outputs against reference calculations over randomized operations. |
| `tests/test_fenwick_advanced.cpp` | Verifies the separately implemented linear-time Fenwick construction and cumulative-frequency order-statistics routine. |
| `benchmarks/benchmark_1d.cpp` | Generates matched 1D point-update and range-sum workloads. Measures construction time, batched update and query latency, total execution time, and throughput. Records checksums and trial-level measurements in CSV format. |
| `benchmarks/benchmark_2d.cpp` | Runs equivalent 2D point-update and rectangle-sum comparisons across three matrix sizes. Records construction time, operation latencies, total execution time, throughput, and checksums in CSV format. |
| `profiling/profile_1d.cpp` | Executes a reproducible mixed 1D workload for whole-process CPU-counter, Cachegrind, and peak-memory profiling. |
| `profiling/profile_2d.cpp` | Executes a reproducible mixed 2D workload for whole-process CPU-counter, Cachegrind, and peak-memory profiling. |

The correctness tests verify outputs for the tested inputs. The benchmark programs measure execution performance under controlled workloads, while the profiling programs provide additional information about CPU instructions, cache behavior, and process memory consumption.

## 3. Requirements

The original experiments use Linux or Ubuntu on WSL. The construction and range-update suite also runs natively on Windows with PowerShell and MSVC or MinGW-w64 GCC. CMake 3.20 or newer is optional for the PowerShell runner.

Required software:

- C++17-compatible compiler, such as `g++`
- Python 3
- Matplotlib for benchmark analysis and graph generation

Optional profiling tools:

- Linux `perf`
- Valgrind Cachegrind
- GNU `/usr/bin/time`

Optional report-building tools:

- XeLaTeX
- Fonts required by the LaTeX report

Run all commands from the project root:

```bash
cd ~/projects/P09FenwickTrees
mkdir -p build results/raw results/processed results/graphs
```

## 4. Correctness testing

Correctness is checked before performance measurements are interpreted.

The tests compare tree outputs against straightforward reference calculations.

### 4.1 Compile

```bash
g++ -std=c++17 -O2 tests/test_correctness.cpp -o build/test_correctness
g++ -std=c++17 -O2 tests/test_fenwick_advanced.cpp -o build/test_fenwick_advanced
```

### 4.2 Execute

```bash
./build/test_correctness
./build/test_fenwick_advanced
```

`test_correctness.cpp` exercises the main 1D and 2D implementations.

`test_fenwick_advanced.cpp` checks linear Fenwick construction and order-statistic functionality.

Randomized tests exercise repeated updates and queries. A passing run establishes correctness for the tested cases, not a formal proof for every possible input.

### 4.3 Construction and range correctness

The third suite checks linear and repeated construction, mixed range operations, and edge cases against reference results. Run all three suites with CMake and CTest on Linux or WSL.

```bash
cmake -S . -B build/range-release -DCMAKE_BUILD_TYPE=Release
cmake --build build/range-release --parallel
ctest --test-dir build/range-release --output-on-failure
```

On Windows, use the same PowerShell runner described in Section 21 with the `-TestsOnly` option.

## 5. Experimental methodology

The benchmarks compare Fenwick Trees and Segment Trees under matched generated workloads.

Each algorithm processes the same type and number of operations for a given configuration.

### 5.1 Experimental configurations

| Variable | 1D experiments | 2D experiments |
|---|---|---|
| Algorithms | Fenwick vs Segment | Fenwick2D vs Segment2D |
| Input sizes | 10,000; 100,000; 1,000,000 | 50×50; 100×100; 250×250 |
| Operations per run | 100,000 | 10,000 |
| Update proportions | 10%, 50%, 90% | 10%, 50%, 90% |
| Trials per configuration | 5 | 5 |
| Timing batch size | 100 operations | 100 operations |

The update proportions define three workload categories.

| Workload | Updates | Queries | Description |
|---|---:|---:|---|
| Read-heavy | 10% | 90% | Most operations calculate sums |
| Balanced | 50% | 50% | Equal proportions of updates and queries |
| Write-heavy | 90% | 10% | Most operations modify values |

For example, a balanced 1D workload contains 50,000 point additions and 50,000 range-sum queries.

The benchmark programs use batches of 100 operations for timing and alternate the execution order of Fenwick and Segment Trees between trials.

Five trials are performed for each configuration.

### 5.2 Performance metrics

| Metric | CSV column | Unit | Meaning | Better result |
|---|---|---|---|---|
| Construction time | `build_ms` | Milliseconds | Time needed to construct the internal data structure | Lower |
| Update latency | `update_ns` | Nanoseconds | Average measured time per point update | Lower |
| Query latency | `query_ns` | Nanoseconds | Average measured time per sum query | Lower |
| Total execution time | `total_ms` | Milliseconds | Time spent executing the benchmark operation workload | Lower |
| Throughput | `throughput_ops_sec` | Operations/second | Number of operations processed per second | Higher |

**Construction time** measures how quickly the tree is prepared from the input.

**Update latency** measures the cost of adding a value to one array element or matrix cell.

**Query latency** measures the cost of calculating a range sum in 1D or rectangle sum in 2D.

**Total execution time** measures the elapsed time for the mixed-operation benchmark workload. It is reported separately from construction time.

**Throughput** measures the number of operations completed per second.

### 5.3 Compile and execute benchmarks

```bash
g++ -std=c++17 -O2 benchmarks/benchmark_1d.cpp -o build/benchmark_1d
g++ -std=c++17 -O2 benchmarks/benchmark_2d.cpp -o build/benchmark_2d

./build/benchmark_1d
./build/benchmark_2d
```

The benchmark programs generate:

```text
results/raw/benchmark_1d.csv
results/raw/benchmark_2d.csv
```

## 6. Raw benchmark CSV files

### 6.1 `benchmark_1d.csv` — Individual 1D measurements

**Location:** `results/raw/benchmark_1d.csv`

Each row represents one algorithm, one array size, one update proportion, and one trial.

| Column | Description |
|---|---|
| `algorithm` | `Fenwick` or `Segment` |
| `n` | Number of elements in the input array |
| `operations` | Total number of operations |
| `update_percent` | Percentage of operations that are point updates |
| `trial` | Trial number |
| `build_ms` | Construction time in milliseconds |
| `update_ns` | Average point-update latency in nanoseconds |
| `query_ns` | Average range-query latency in nanoseconds |
| `total_ms` | Total benchmark operation time in milliseconds |
| `throughput_ops_sec` | Operations processed per second |
| `checksum` | Aggregate of query results used for cross-checking |

The file contains 90 measurement rows:

```text
3 input sizes × 3 update ratios × 5 trials × 2 algorithms = 90
```

Matching checksums are a useful consistency check, but they do not prove that every individual query result is correct.

### 6.2 `benchmark_2d.csv` — Individual 2D measurements

**Location:** `results/raw/benchmark_2d.csv`

This file contains corresponding measurements for the 2D implementations.

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

The `rows` and `cols` fields specify the matrix dimensions.

For example, a 250×250 matrix contains 62,500 cells.

The file contains 90 measurement rows.

## 7. Processed CSV files and statistical analysis

The Python analysis script combines trial-level measurements, calculates statistics, compares algorithms, and generates graphs.

### 7.1 Generate summaries and graphs

```bash
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install matplotlib

python3 scripts/analyze_results.py
```

Generated files:

```text
results/processed/benchmark_1d_summary.csv
results/processed/benchmark_2d_summary.csv
results/processed/benchmark_1d_speedups.csv
results/processed/benchmark_2d_speedups.csv
results/graphs/*.png
```

### 7.2 `benchmark_1d_summary.csv`

**Purpose:** Summarizes the five trials of every 1D benchmark configuration.

The file contains:

```text
dimension
size
operations
update_percent
algorithm
trials

build_ms_mean
build_ms_std

update_ns_mean
update_ns_std

query_ns_mean
query_ns_std

total_ms_mean
total_ms_std

throughput_ops_sec_mean
throughput_ops_sec_std
```

The `_mean` suffix indicates the arithmetic average across trials.

The `_std` suffix indicates the sample standard deviation.

Standard deviation describes how much the measurements vary around their mean.

A larger standard deviation relative to the mean indicates less consistent measurements.

The file contains 18 grouped results.

### 7.3 `benchmark_2d_summary.csv`

**Purpose:** Summarizes the five trials of every 2D benchmark configuration.

It contains the same statistical fields as the 1D summary.

The `size` field identifies matrix dimensions such as `50x50`, `100x100`, and `250x250`.

The file contains 18 grouped results.

### 7.4 `benchmark_1d_speedups.csv`

**Purpose:** Directly compares average 1D Fenwick and Segment Tree performance.

For every metric, the file contains the Fenwick measurement, Segment Tree measurement, and Fenwick advantage ratio.

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

For time-based metrics:

```text
Fenwick advantage = Segment time / Fenwick time
```

For throughput:

```text
Fenwick advantage = Fenwick throughput / Segment throughput
```

Interpretation:

| Advantage ratio | Meaning |
|---|---|
| Greater than 1× | Fenwick performs better |
| Equal to 1× | Equal measured performance |
| Less than 1× | Segment Tree performs better |

For example, a 3× update-latency advantage means the Segment Tree takes three times as long per update as the Fenwick Tree.

### 7.5 `benchmark_2d_speedups.csv`

**Purpose:** Directly compares average 2D Fenwick and Segment Tree performance.

The calculations and interpretation are the same as for the 1D speedup file.

## 8. Experiment 1 — Construction time

**Research question:** Which data structure takes less time to construct?

Construction occurs before the measured update/query workload.

The experiment measures how quickly each implementation builds its internal representation from the input.

### Method

Both algorithms are constructed for the same input size.

Construction time is measured in milliseconds.

Five trials are performed for each configuration.

### Graphs

| Graph | Description |
|---|---|
| [`1d_build_ms_updates_10.png`](results/graphs/1d_build_ms_updates_10.png) | 1D construction, read-heavy configuration |
| [`1d_build_ms_updates_50.png`](results/graphs/1d_build_ms_updates_50.png) | 1D construction, balanced configuration |
| [`1d_build_ms_updates_90.png`](results/graphs/1d_build_ms_updates_90.png) | 1D construction, write-heavy configuration |
| [`2d_build_ms_updates_10.png`](results/graphs/2d_build_ms_updates_10.png) | 2D construction, read-heavy configuration |
| [`2d_build_ms_updates_50.png`](results/graphs/2d_build_ms_updates_50.png) | 2D construction, balanced configuration |
| [`2d_build_ms_updates_90.png`](results/graphs/2d_build_ms_updates_90.png) | 2D construction, write-heavy configuration |

### Findings

Segment Trees generally showed lower construction times in the tested configurations.

The benchmarked Fenwick constructors use repeated point updates, whereas the Segment Tree constructors recursively build their internal representations.

The separately implemented linear-time 1D Fenwick constructor was not substituted into the main benchmark.

The construction results therefore describe the tested construction strategies rather than every possible implementation.

## 9. Experiment 2 — Point-update latency

**Research question:** Which implementation modifies a value faster?

A point update adds a value to one array element or matrix cell.

In 1D:

```text
A[i] += delta
```

In 2D:

```text
A[row][col] += delta
```

After the update, the data structure must maintain correct aggregate sums.

### Method

Point-update latency is measured in nanoseconds.

Both implementations execute corresponding updates for each workload configuration.

### Graphs

| Graph | Description |
|---|---|
| [`1d_update_ns_updates_10.png`](results/graphs/1d_update_ns_updates_10.png) | 1D update latency, read-heavy |
| [`1d_update_ns_updates_50.png`](results/graphs/1d_update_ns_updates_50.png) | 1D update latency, balanced |
| [`1d_update_ns_updates_90.png`](results/graphs/1d_update_ns_updates_90.png) | 1D update latency, write-heavy |
| [`2d_update_ns_updates_10.png`](results/graphs/2d_update_ns_updates_10.png) | 2D update latency, read-heavy |
| [`2d_update_ns_updates_50.png`](results/graphs/2d_update_ns_updates_50.png) | 2D update latency, balanced |
| [`2d_update_ns_updates_90.png`](results/graphs/2d_update_ns_updates_90.png) | 2D update latency, write-heavy |

### Findings

Fenwick Trees had lower measured point-update latency in all nine configurations in both dimensions.

The observed advantage ranged from approximately 2.24–5.15× in 1D and 1.39–3.42× in 2D.

The Fenwick implementations use compact low-bit loops, whereas the Segment Tree implementations use recursive traversal.

These implementation differences may contribute to the measured performance gap, although the benchmark does not isolate each individual source of overhead.

## 10. Experiment 3 — Query latency

**Research question:** Which implementation calculates sums faster?

In 1D, a range query calculates:

```text
sum(A[left ... right])
```

In 2D, a rectangle query calculates:

```text
sum(A[row1 ... row2][col1 ... col2])
```

### Method

Average query latency is measured in nanoseconds.

The 1D benchmarks use interval-sum queries.

The 2D benchmarks use rectangle-sum queries.

### Graphs

| Graph | Description |
|---|---|
| [`1d_query_ns_updates_10.png`](results/graphs/1d_query_ns_updates_10.png) | 1D query latency, read-heavy |
| [`1d_query_ns_updates_50.png`](results/graphs/1d_query_ns_updates_50.png) | 1D query latency, balanced |
| [`1d_query_ns_updates_90.png`](results/graphs/1d_query_ns_updates_90.png) | 1D query latency, write-heavy |
| [`2d_query_ns_updates_10.png`](results/graphs/2d_query_ns_updates_10.png) | 2D query latency, read-heavy |
| [`2d_query_ns_updates_50.png`](results/graphs/2d_query_ns_updates_50.png) | 2D query latency, balanced |
| [`2d_query_ns_updates_90.png`](results/graphs/2d_query_ns_updates_90.png) | 2D query latency, write-heavy |

### Findings

Fenwick Trees had lower measured query latency in every tested configuration.

The 1D advantage ranged from approximately 1.57–4.40×.

The 2D advantage ranged from approximately 1.67–3.36×.

Fenwick Trees answer range queries using prefix sums and subtraction.

Segment Trees recursively identify nodes whose intervals or rectangles cover the requested region.

## 11. Experiment 4 — Total execution time

**Research question:** Which implementation completes the mixed workload faster?

Individual operation latencies measure the cost of one operation type.

Total execution time measures the overall cost of executing the generated workload.

### Method

Each algorithm executes the matched workload for a given configuration.

Elapsed operation time is measured in milliseconds.

### Graphs

| Graph | Description |
|---|---|
| [`1d_total_ms_updates_10.png`](results/graphs/1d_total_ms_updates_10.png) | 1D total time, read-heavy |
| [`1d_total_ms_updates_50.png`](results/graphs/1d_total_ms_updates_50.png) | 1D total time, balanced |
| [`1d_total_ms_updates_90.png`](results/graphs/1d_total_ms_updates_90.png) | 1D total time, write-heavy |
| [`2d_total_ms_updates_10.png`](results/graphs/2d_total_ms_updates_10.png) | 2D total time, read-heavy |
| [`2d_total_ms_updates_50.png`](results/graphs/2d_total_ms_updates_50.png) | 2D total time, balanced |
| [`2d_total_ms_updates_90.png`](results/graphs/2d_total_ms_updates_90.png) | 2D total time, write-heavy |

### Findings

Fenwick Trees completed the measured workloads faster in all tested configurations.

| Dimension | Total-time Fenwick advantage |
|---|---|
| 1D | Approximately 1.82–4.17× |
| 2D | Approximately 1.60–3.35× |

The advantage varies with input size and update/query mixture.

It should not be interpreted as a fixed speedup for every workload.

## 12. Experiment 5 — Throughput

**Research question:** How many operations can each implementation process per second?

Throughput measures the amount of work completed per second.

Unlike latency, higher throughput indicates better performance.

### Method

Throughput is calculated from the number of benchmark operations and the measured execution time.

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

Fenwick Trees achieved higher measured throughput in the tested configurations.

This agrees with their lower total execution times.

Throughput and total execution time are related measurements, not independent evidence of different performance properties.

## 13. Experiment 6 — Input-size scalability

**Research question:** How does performance change as the input grows?

Larger inputs can increase memory access costs and affect cache behavior.

The experiment compares the algorithms at several input sizes rather than relying on a single configuration.

### Input sizes

1D:

```text
10,000 elements
100,000 elements
1,000,000 elements
```

2D:

```text
50 × 50 cells
100 × 100 cells
250 × 250 cells
```

### Relevant results

```text
results/processed/benchmark_1d_speedups.csv
results/processed/benchmark_2d_speedups.csv
```

The update-latency, query-latency, and total-time graphs also display the effect of input size.

### Findings

Fenwick Trees retained a measured advantage throughout the tested size ranges.

However, the magnitude of the advantage changed across configurations.

The relative advantage in 1D generally became smaller at one million elements than in some smaller configurations.

The results show why performance should be examined across multiple input sizes and workload mixtures.

## 14. Experiment 7 — CPU and instruction profiling

**Research question:** How much processor work does each implementation require?

Elapsed time measures how long a program takes.

CPU profiling provides additional information about the processor instructions and clock cycles involved.

### Tools and commands

Compile:

```bash
g++ -std=c++17 -O2 profiling/profile_1d.cpp -o build/profile_1d
g++ -std=c++17 -O2 profiling/profile_2d.cpp -o build/profile_2d
```

Example 1D profiling:

```bash
perf stat -r 5 \
  -e cycles,instructions,cache-references,cache-misses \
  ./build/profile_1d fenwick 100000 50 100000
```

Example 2D profiling:

```bash
perf stat -r 5 \
  -e cycles,instructions,cache-references,cache-misses \
  ./build/profile_2d fenwick 250 250 50 100000
```

Repeat with `segment`.

### Metrics

| Metric | Meaning |
|---|---|
| CPU cycles | Processor clock cycles counted during execution |
| Instructions | Retired machine instructions |
| Instructions per cycle (IPC) | Average instructions retired per CPU cycle |
| Cache references | Hardware-reported cache reference events |
| Cache misses | Hardware-reported cache miss events |

```text
IPC = Instructions / CPU cycles
```

A higher IPC indicates more instructions retired per cycle, but does not automatically mean lower execution time.

### 14.1 Measured 1D profiling results

| Metric | Fenwick | Segment Tree |
|---|---:|---:|
| CPU cycles | 36,871,900 | 47,425,949 |
| Instructions | 60,645,178 | 67,718,859 |
| IPC | 1.64 | 1.43 |
| Cache references | 2,542,967 | 1,917,881 |
| Cache misses | 177,298 | 156,483 |
| Cache miss rate | 6.97% | 8.16% |

Fenwick used approximately 22.3% fewer CPU cycles and 10.4% fewer instructions in this profiling comparison.

It recorded more absolute cache misses but a lower cache miss rate.

### 14.2 Measured 2D profiling results

| Metric | Fenwick2D | Segment2D |
|---|---:|---:|
| CPU cycles | 51,983,673 | 90,913,465 |
| Instructions | 106,227,450 | 116,912,825 |
| Cache references | 2,913,013 | 7,839,828 |
| Cache misses | 177,842 | 215,810 |

Fenwick2D used fewer measured CPU cycles, instructions, cache references, and cache misses in this configuration.

The `perf` wall-clock results were less consistent, so the CPU-cycle difference should not be interpreted as an identical elapsed-time speedup.

Profiling covers the complete process, including initialization and output.

## 15. Experiment 8 — Cache efficiency

**Research question:** How do the implementations behave when accessing memory?

Processors use caches to reduce the cost of accessing main memory.

A cache miss occurs when requested data is not available at the relevant cache level.

The project examines cache behavior using hardware counters and Cachegrind.

### Method

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

Fenwick2D produced approximately 50.3% fewer simulated L1 data-read misses in this profiling configuration.

Cachegrind simulates cache behavior. Its results are not identical to the hardware events reported by `perf`.

## 16. Experiment 9 — Memory consumption

**Research question:** How much physical memory does each program use?

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

Fenwick used approximately 676 KiB less peak memory in the 1D comparison and 1,508 KiB less in the 2D comparison.

These values represent whole-process memory, not the exact allocation size of the tree alone.

## 17. How to interpret the graphs

All benchmark graphs are stored in:

```text
results/graphs/
```

The filename format is:

```text
<dimension>_<metric>_updates_<percentage>.png
```

For example:

```text
1d_update_ns_updates_50.png
```

means:

- `1d`: One-dimensional benchmark
- `update_ns`: Point-update latency in nanoseconds
- `updates_50`: Balanced workload containing 50% updates

The horizontal axis represents input size.

The vertical axis represents the measured performance metric.

Bars represent averages across five trials, with error bars indicating sample standard deviation.

For construction time, update latency, query latency, and total execution time, shorter bars indicate better performance.

For throughput, taller bars indicate better performance.

The report figures may use different visual styling from the analysis script's default graphs, but they represent measurements from the same benchmark data.

## 18. Complete graph index

The analysis script generates 30 graphs: five metrics, three update proportions, and two dimensions.

### 18.1 Construction-time graphs

| Filename | Comparison |
|---|---|
| `1d_build_ms_updates_10.png` | 1D construction, 10% updates |
| `1d_build_ms_updates_50.png` | 1D construction, 50% updates |
| `1d_build_ms_updates_90.png` | 1D construction, 90% updates |
| `2d_build_ms_updates_10.png` | 2D construction, 10% updates |
| `2d_build_ms_updates_50.png` | 2D construction, 50% updates |
| `2d_build_ms_updates_90.png` | 2D construction, 90% updates |

### 18.2 Point-update latency graphs

| Filename | Comparison |
|---|---|
| `1d_update_ns_updates_10.png` | 1D update latency, 10% updates |
| `1d_update_ns_updates_50.png` | 1D update latency, 50% updates |
| `1d_update_ns_updates_90.png` | 1D update latency, 90% updates |
| `2d_update_ns_updates_10.png` | 2D update latency, 10% updates |
| `2d_update_ns_updates_50.png` | 2D update latency, 50% updates |
| `2d_update_ns_updates_90.png` | 2D update latency, 90% updates |

### 18.3 Query-latency graphs

| Filename | Comparison |
|---|---|
| `1d_query_ns_updates_10.png` | 1D query latency, 10% updates |
| `1d_query_ns_updates_50.png` | 1D query latency, 50% updates |
| `1d_query_ns_updates_90.png` | 1D query latency, 90% updates |
| `2d_query_ns_updates_10.png` | 2D query latency, 10% updates |
| `2d_query_ns_updates_50.png` | 2D query latency, 50% updates |
| `2d_query_ns_updates_90.png` | 2D query latency, 90% updates |

### 18.4 Total execution-time graphs

| Filename | Comparison |
|---|---|
| `1d_total_ms_updates_10.png` | 1D total execution time, 10% updates |
| `1d_total_ms_updates_50.png` | 1D total execution time, 50% updates |
| `1d_total_ms_updates_90.png` | 1D total execution time, 90% updates |
| `2d_total_ms_updates_10.png` | 2D total execution time, 10% updates |
| `2d_total_ms_updates_50.png` | 2D total execution time, 50% updates |
| `2d_total_ms_updates_90.png` | 2D total execution time, 90% updates |

### 18.5 Throughput graphs

| Filename | Comparison |
|---|---|
| `1d_throughput_ops_sec_updates_10.png` | 1D throughput, 10% updates |
| `1d_throughput_ops_sec_updates_50.png` | 1D throughput, 50% updates |
| `1d_throughput_ops_sec_updates_90.png` | 1D throughput, 90% updates |
| `2d_throughput_ops_sec_updates_10.png` | 2D throughput, 10% updates |
| `2d_throughput_ops_sec_updates_50.png` | 2D throughput, 50% updates |
| `2d_throughput_ops_sec_updates_90.png` | 2D throughput, 90% updates |

## 19. Reproducing the experiments

The following sequence reproduces the correctness tests, benchmarks, and statistical analysis.

### Step 1 — Create output directories

```bash
mkdir -p build results/raw results/processed results/graphs
```

### Step 2 — Compile correctness tests

```bash
g++ -std=c++17 -O2 tests/test_correctness.cpp -o build/test_correctness
g++ -std=c++17 -O2 tests/test_fenwick_advanced.cpp -o build/test_fenwick_advanced
```

### Step 3 — Run correctness tests

```bash
./build/test_correctness
./build/test_fenwick_advanced
```

### Step 4 — Compile benchmarks

```bash
g++ -std=c++17 -O2 benchmarks/benchmark_1d.cpp -o build/benchmark_1d
g++ -std=c++17 -O2 benchmarks/benchmark_2d.cpp -o build/benchmark_2d
```

Step 5 — Run benchmarks

./build/benchmark_1d
./build/benchmark_2d

Step 6 — Generate summaries and graphs

python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install matplotlib
python3 scripts/analyze_results.py

Step 7 — Inspect generated files

ls results/raw/
ls results/processed/
ls results/graphs/

To open the graph folder in Windows from WSL:

explorer.exe "$(wslpath -w "$PWD/results/graphs")"

## 20. Main findings and limitations

In the original 1D and 2D point-update benchmarks, Fenwick Trees completed the measured workloads faster than the corresponding Segment Trees.

Across the nine benchmark configurations in each dimension, Fenwick Trees achieved lower measured point-update latency, query latency, and total execution time.

The total execution-time advantage ranged from approximately 1.82–4.17× in 1D and 1.60–3.35× in 2D.

In the original 1D benchmarks, the O(n) Segment Tree built faster on average than the O(n log n) Fenwick Tree (built using repeated point updates) in 8 of 9 configurations. The original 2D Segment Tree also built faster on average in 8 of 9 configurations against the 2D Fenwick Tree built using repeated point updates.

In the newer 1D construction experiment, the O(n) Fenwick Tree (linear build) had the lowest median build time at all three tested sizes, followed by the O(n) Segment Tree and the O(n log n) Fenwick Tree. At one million elements, their median build times were 3.70 ms, 8.08 ms, and 14.05 ms, respectively.

CPU and cache profiling showed differences in instruction counts, processor cycles, cache behavior, and peak process memory. These measurements help explain the observed performance differences, but profiling includes whole-process activity and does not isolate every source of overhead.

The results are specific to the implemented algorithms, tested workloads, compiler settings, and hardware environment. They do not establish that Fenwick Trees are faster for every possible operation, implementation, or input distribution.



## 21. Construction and range-update experiments

The extension uses six trials per configuration. Workload experiments use 100,000 operations at 10%, 50%, and 90% updates on arrays of 10,000, 100,000, and 1,000,000 elements. Each pair uses the same input and seeded operation sequence. Construction is timed separately from replay. The results include medians, quartiles, tree storage, and paired timing ratios where applicable.

| Experiment | Implementations | Result at 1,000,000 elements |
|---|---|---|
| Construction | O(n) Fenwick Tree, O(n) iterative Segment Tree, O(n log n) Fenwick Tree | Median builds took 3.70 ms, 8.08 ms, and 14.05 ms, respectively |
| A. Point addition and range sum | Linear-build Fenwick and iterative Segment Tree | Median replay at 50% updates was 4.59 ms versus 14.15 ms |
| B. Range addition and range sum | Two-BIT Fenwick and lazy Segment Tree | At 50% updates, median replay was 18.56 vs 43.95 ms for short ranges and 20.70 vs 65.87 ms for wide ranges |
| C. Range assignment and range sum | Lazy Segment Tree | Median replay at 50% updates was 41.24 ms for short ranges and 57.17 ms for wide ranges |

Experiment C measures assignment separately because overwriting an interval can require different deltas at each position. See the [methodology and commands](docs/range_update_experiments.md) and the [recorded analysis and graphs](results/range_windows_01/analysis/).

### 21.1 Run the extension

On Windows, run from the project root with a compiler available in your shell.

```powershell
py -m pip install matplotlib
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\run_range_experiments.ps1
```

On Linux or WSL, after the build and correctness checks in Section 4.3, choose a fresh output directory.

```bash
./build/range-release/benchmark_range_structures results/range_run_01
python3 scripts/analyze_range_structures.py results/range_run_01
```

### 21.2 Cache efficiency and Windows profiling

Sections 14–15 cover whole-process Linux hardware counters and Cachegrind simulation for the original trees. Both absolute cache misses and misses per reference matter. They can rank the same measurements differently, and process-level counts include work outside the tree operations.

The extension's Windows profiling measured thread cycles for the workload replay. The Windows hardware-counter trace could not be collected because recording was denied, so there are no new Windows cache-miss or instruction counts. The [profiling notes](docs/range_hardware_profiling.md) explain the measurement and reproduction commands.
