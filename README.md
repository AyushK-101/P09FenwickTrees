# P09: Fenwick Trees vs. Segment Trees

**Course:** CS G526 – Advanced Algorithms and Complexity

**Project Type:** Individual

## 1. Overview

This project investigates the theoretical and practical performance differences between Fenwick Trees (Binary Indexed Trees) and Segment Trees in one-dimensional and two-dimensional range query processing.

The study focuses on bit manipulation, memory layout, execution efficiency, cache behavior and hardware performance profiling.

## 2. Implementations

The project will implement:

- 1D Fenwick Tree
- 1D Segment Tree
- 2D Fenwick Tree
- 2D Segment Tree
- Fenwick Tree Binary Lifting for order statistics

## 3. Performance Evaluation

The following metrics will be investigated:

- Execution time
- Point update latency
- Prefix and range query latency
- CPU cycles and instructions
- Cache references and cache misses
- Branch instructions and branch mispredictions
- Memory consumption

## 4. Experimental Workloads

Three workload configurations will be evaluated:

| Workload | Updates | Queries |
|---|---:|---:|
| Write-heavy | 90% | 10% |
| Balanced | 50% | 50% |
| Read-heavy | 10% | 90% |

Random, sequential and localized access patterns will also be investigated.

## 5. Technologies

- C++ – Algorithm implementations and benchmarking
- GCC – Compilation and optimization
- Linux / WSL2 – Execution and profiling environment
- Linux perf – Hardware performance counters
- Cachegrind – Cache simulation
- Python – Experimental analysis and visualization
- Git and GitHub – Version control and project documentation

## 6. Repository Structure

See the source directories, benchmarks, profiling scripts, results and documentation folders.

## 7. Building and Running

Build and execution instructions will be documented as the implementations are completed.

## 8. Experimental Results

Benchmark measurements, CSV datasets, performance graphs and analysis will be added here.

## 9. Reproducibility

The final documentation will specify the hardware configuration, compiler version, optimization flags, dataset sizes and experimental methodology.

## 10. Author

Individual academic project – CS G526, Advanced Algorithms and Complexity.