# Construction and range-update results

The tables report medians across six trials.

## Construction at 1,000,000 elements

| Builder | Build (ms) | Tree storage (MiB) |
|---|---:|---:|
| Fenwick (linear) | 3.703 | 7.63 |
| Segment (iterative) | 8.083 | 15.26 |
| Fenwick (repeated) | 14.050 | 7.63 |

The two Fenwick builders use the same representation. Their build-time difference measures the change in construction procedure, including its initialization work.

## Workload timings at 1,000,000 elements and 50% updates

| Update | Span | Tree | Update (ns) | Query (ns) | Replay (ms) |
|---|---|---|---:|---:|---:|
| Addition | point | Fenwick (linear) | 129.0 | 144.1 | 4.587 |
| Addition | point | Segment (iterative) | 257.6 | 191.9 | 14.152 |
| Addition | short | Fenwick (two BITs) | 236.0 | 221.5 | 18.556 |
| Addition | short | Segment (lazy) | 453.0 | 497.1 | 43.951 |
| Addition | wide | Fenwick (two BITs) | 275.1 | 218.6 | 20.699 |
| Addition | wide | Segment (lazy) | 811.7 | 597.1 | 65.871 |
| Assignment | short | Segment (lazy) | 436.2 | 472.6 | 41.239 |
| Assignment | wide | Segment (lazy) | 722.8 | 470.4 | 57.174 |

## Replay time across all paired configurations

Configurations are classified by the median of the six paired comparator/baseline duration ratios.

| Workload | Fenwick lower | Segment lower | Equal |
|---|---:|---:|---:|
| Point addition | 9 | 0 | 0 |
| Short range addition | 9 | 0 | 0 |
| Wide range addition | 9 | 0 | 0 |

Replay timing excludes construction. Range-addition timings reflect both the boundary updates in the two-BIT tree and the recursive traversal and tag pushes in the lazy tree. Partial queries can trigger those pushes. The experiment compares these implementations under the sampled interval distributions.

Assignment timings show how the lazy implementation behaves as the interval distribution and update proportion vary. A Fenwick point-addition tree would need to derive a separate delta for each overwritten value.

Operation latency includes the clock measurement. Use the empty timer interval reported in the summaries when interpreting small differences.
