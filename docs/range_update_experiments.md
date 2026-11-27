# Linear construction and 1D range-update experiments

## What the original code builds

| Original implementation | Build used by the benchmark and profiler | Build time | Tree storage |
|---|---|---|---|
| 1D Fenwick | Zero-fill and call point addition once at every index | Θ(n log(n + 1)) | n + 1 sum slots |
| 1D Segment | Copy n leaves and combine n − 1 parents from the bottom up | Θ(n) | 2n sum slots |
| 2D Fenwick | Zero-fill the grid and call point addition once at every cell | Θ(rows × cols × log(rows + 1) × log(cols + 1)) | (rows + 1) × (cols + 1) slots, split into row vectors |
| 2D Segment | Place leaves, combine column parents for each leaf row, then combine row parents for every column slot | Θ(rows × cols) | 4 × rows × cols slots, split into row vectors |

The Fenwick size-only constructors take linear time to allocate and zero-fill. The repeated-update costs above describe building from populated input. During 1D repeated construction, slot `i` receives exactly `lowBit(i)` contributions. For a power-of-two array, each power-of-two slot size below n contributes n/2 total writes across all slots of that size. The total is Θ(n log(n + 1)), with the same order for general n by bounding it between consecutive powers of two. The 2D nested update counts factor into one sum of low bits per dimension. A point update or range query takes O(log(n + 1)) in the original 1D trees and O(log(rows + 1) × log(cols + 1)) in the original 2D trees.

The original linear-build demo visits slots in ascending order, adds the current input element to the contributions already present, then transfers the completed sum to `i + lowBit(i)` if the parent exists. Each transfer goes forward. The invariant is that slot `i` ends up holding the sum of its `lowBit(i)` consecutive elements. The old advanced test checks every prefix of a signed array of length 1,000. The extension shares that builder with the original demo and order-statistic example and exposes it through the standard tree's array constructor. The size constructor and original repeated-update callers still use the same update API.

## How the original numbers were collected

- The original 1D benchmark uses 10,000, 100,000 and 1,000,000 elements, with 100,000 operations per trial. The original 2D benchmark uses square grids of 50, 100 and 250 per side, with 10,000 operations. Both use 10%, 50% and 90% updates and five trials.
- Input generation uses seed 42 and workload generation uses seed 12345. Each pair sees the same input and operations. Execution order alternates across trials and a sum of query answers is compared between structures.
- Homogeneous batches of 100 operations are shuffled. Consecutive batches of the same type can be timed together. All five trials reuse the same trace for a configuration. This measures timing variation for that trace and gives update/query runs more locality than individually shuffled operations. With five trials, one implementation runs first three times. The 1D benchmark uses the lesser of two random endpoints for updates, so smaller indices are more likely to be written. The 2D benchmark samples each update coordinate uniformly.
- Construction includes allocation and zero-initialization. Original total time and throughput describe the workload replay and include batch bookkeeping and checksum accumulation. The build comparisons confound data structure and build algorithm. Summing answers detects some divergences and can miss errors that cancel out.
- The profiling drivers run one algorithm per process and individually shuffle operations. Their updates use the lesser sampled endpoint in each dimension. The recorded `perf stat` counts and `/usr/bin/time -v` peak RSS include trace generation, construction, execution and process overhead. The checked-in Cachegrind annotations record instruction references. These measurements cannot isolate operation-level memory use, cache misses or branch mispredictions.
- The original Python analysis groups runs by dimensions, operation count, ratio and algorithm and reports means, sample standard deviations and ratios of means. It does not verify checksum equality or summarize paired trial ratios.

## Proposal status

| Proposal item | Repository status |
|---|---|
| Low-bit extraction, both Fenwick traversals, bit lifting and 2D traversal | Implemented in the original source. Order statistics require nonnegative frequencies. |
| Pointer-based segment-tree comparison | Both original implementations use array indexing. The 2D structures use vectors of rows, with a separate allocation per row. |
| Range addition and assignment with lazy tags | Added in 1D. The existing 2D point-addition and rectangle-sum comparison covers grids through 250 × 250. |
| Benchmarks through 10⁷ in 1D and 4,000 × 4,000 in 2D | Existing measurements stop at 10⁶ and 250 × 250. The new 1D suite also stops at 10⁶. |
| Hardware profiling and exact structure memory | Original process-level observations are in `results/raw`. The extension reports retained tree-vector capacity in bytes. |
| Range minimum/maximum and cycle-level claims | The tested operations are sums. Source inspection gives traversal and storage costs. CPU cycle counts for individual instructions, branch behavior and cache effects would need targeted measurements. |

## What each new experiment measures

| Group | Implementations | Update interval |
|---|---|---|
| Construction | Linear Fenwick, repeated-update Fenwick, iterative Segment | Initial array only |
| A | Linear Fenwick and iterative Segment | One uniformly selected element |
| B | Two-BIT Fenwick and lazy Segment | Short and wide, measured separately |
| C | Lazy Segment | Short and wide, measured separately |

The two-BIT tree builds from the differences `D[i] = A[i] − A[i−1]` and their weighted values `(i−1)D[i]`, with `A[0] = 0`. Its prefix sum through `x` is `x × prefix(D, x) − prefix(weighted, x)`. A range addition changes the two difference boundaries, with no right boundary stored after the last element. All four main 1D structures use inclusive, 1-based indices. Arithmetic and intermediates must fit in signed 64 bits.

The lazy tree builds a flat preorder array of `2n − 1` nodes in Θ(n) time. Each node stores its sum and one deferred action. Assignment replaces the previous action. Addition increments its value, including when the action is a pending assignment. Partial updates and queries push the action as they descend. Addition, assignment and range sum each take O(log(n + 1)) in the worst case, and full-array operations can return or apply at the root.

Range assignment overwrites each value with a constant. The required additive change at index `i` is `constant − A[i]`, which depends on the current value. A pair of boundary additions cannot cancel arbitrary interior differences. Emulating an assignment with ordinary Fenwick point queries and point additions can take O(k log(n + 1)) for an interval of length k, so experiment C has no Fenwick comparator with the same operation and complexity.

Each workload configuration runs 100,000 individually shuffled operations at each of 10%, 50% and 90% updates, with six trials and a distinct deterministic trace per trial. Array sizes are 10,000, 100,000 and 1,000,000. Short updates have a uniform start and a 0–31 element offset, clipped at the end. Wide updates and all queries sort two independently sampled uniform endpoints. B and C share seeds, ranges and update values for the corresponding trials. The paired trees run in alternating order. Each of the six permutations of construction method order is used once. Standard C++ distributions and shuffle provide repeatability on the same standard-library implementation. Different standard libraries may produce different traces.

Construction timing includes tree allocation and initialization and excludes input generation, destruction and validation. Construction trials verify every point against the input. For workloads, a fresh tree replays the stream with only outer timestamps for total time and throughput. That timed section includes iteration, dispatch and query-answer hashing. A second fresh tree replays the same stream with operation-level timestamps to estimate mean update and query latency. Those means include timer overhead. The CSV records a back-to-back timer reading to show its scale. Each replay hashes all query answers in order plus the final full-array sum. The benchmark checks replay equality and paired-implementation equality. The differential tests check answers directly against arrays.

The CSV also records backing-vector capacity multiplied by element size, including node padding for the lazy tree. This measurement excludes allocator metadata, stack, input, operation trace and process overhead. The analysis reports medians and quartiles across trials, plus matched comparator/baseline ratios where there is a second implementation. For duration metrics, a paired ratio above 1 means the comparator takes longer. For throughput, a ratio above 1 means the comparator handles more operations per second.

Read construction and workload times independently. A repeated/linear Fenwick construction ratio compares two builders for the same representation. Experiment A's workload timings compare the two standard trees once built. Experiment B also varies the update span, which affects how many tree segments the lazy implementation visits and how often pending work is pushed by a query. In experiment C, changing the update proportion or span changes the state the queries encounter. Because construction is outside the timed replay, construction differences cannot account for differences in replay throughput. The design compares complete implementations, so their timings alone cannot isolate a CPU-level cause.

## Run the extension

Run from the repository root. A new output directory is required for every benchmark run. The analysis creates four summary CSVs, three paired-ratio CSVs, 16 graphs and a findings page inside that directory. The legacy README, report and result paths remain usable for reproducing the original study.

**WSL Ubuntu or Linux**

```bash
cmake -S . -B build/range-release -DCMAKE_BUILD_TYPE=Release &&
cmake --build build/range-release --parallel &&
ctest --test-dir build/range-release --output-on-failure &&
  ./build/range-release/benchmark_range_structures results/range_run_01 &&
  python3 scripts/analyze_range_structures.py results/range_run_01
```

For an existing WSL setup with GCC and no CMake, build and test directly.

```bash
set -e
mkdir -p build
for test in test_correctness test_fenwick_advanced test_range_structures; do
  g++ -std=c++17 -O2 "tests/$test.cpp" -o "build/$test"
  "./build/$test"
done
g++ -std=c++17 -O2 benchmarks/benchmark_range_structures.cpp -o build/benchmark_range_structures
./build/benchmark_range_structures results/range_run_01
python3 scripts/analyze_range_structures.py results/range_run_01
```

**Native Windows PowerShell**

Use Microsoft C++ Build Tools in an x64 Developer PowerShell, or use a native MinGW-w64 GCC installation. For the full run, install Matplotlib in your Windows Python environment with `py -m pip install matplotlib`. Then run from the repository root.

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\run_range_experiments.ps1
```

The runner compiles Windows executables and runs all three correctness suites before benchmarking. It writes to a new timestamped directory under `results` by default and records the run environment. The runner needs PowerShell 5.1 or later and can run directly with the compiler, independently of CMake. Pass `-TestsOnly` to run the C++ tests without Python. Pass `-OutputDirectory` to name the results directory, `-Compiler` to select `cl.exe` or `g++.exe`, or `-PythonExecutable` to select a specific Python installation.

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\run_range_experiments.ps1 -Compiler 'C:\msys64\ucrt64\bin\g++.exe' -TestsOnly
```

Graphing requires Python 3.8+ and Matplotlib 3.6+. The C++ code uses the standard library, including `std::filesystem`. The old 1D profiling driver now uses standard includes. Linux `perf` and Valgrind commands in the original README are separate from this Windows benchmark pipeline.
