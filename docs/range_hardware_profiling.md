# Profile the range workloads

The benchmark executable accepts `--profile MODE REPEATS` in addition to its usual CSV-producing invocation. Each process runs one implementation. Modes are `point-fenwick`, `point-segment`, `range-fenwick`, `range-lazy`, and `assign-lazy`. The first two use point addition, the next two use wide range addition, and the last uses wide assignment. The fixed workload has 1,000,000 elements, 100,000 operations per repeat, 50% updates, and workload seed 12445. One tree carries state between repeats, amortizing setup. Both modes in each pair should print the same checksum at the same repeat count.

## Windows thread-cycle measurements

The Windows executable reports `thread_cycles` using [QueryThreadCycleTime](https://learn.microsoft.com/en-us/windows/win32/api/realtimeapiset/nf-realtimeapiset-querythreadcycletime) when the read succeeds. The bracket starts after tree construction and covers the repeated replays, including timing and checksum bookkeeping. Run the following in the project root after building the Windows benchmark.

```powershell
$env:Path = "$(Resolve-Path .\build\gcc\mingw64\bin);$env:Path"
.\scripts\measure_thread_cycles.ps1 `
    -Executable .\build\windows-mingw\benchmark_range_structures.exe `
    -OutputCsv build\thread_cycles_recreated.csv
```

The script runs six trials per mode, with 30 passes (3,000,000 operations) per process. It alternates the order of each compared pair and validates matching checksums. Thread cycles include user and kernel execution and should not be converted into elapsed time. The CSV records Windows cycle counts. Instructions and cache misses require separate PMU access.

## Windows with Windows Performance Recorder

The profile records retired instructions, cycles, and last-level cache misses on context switches. Check counter names with `wpr -pmcsources`. Run these commands in an elevated PowerShell from the repository root, with no other WPR recording active. WPR start and stop must both succeed before an ETL can be used.

```powershell
$exe = '.\build\windows-mingw\benchmark_range_structures.exe'
$env:Path = "$(Resolve-Path .\build\gcc\mingw64\bin);$env:Path"
$profile = "$PWD\profiling\range_pmu.wprp!P09"
$run = Join-Path $PWD ('build\pmu_' + (Get-Date -Format yyyyMMdd_HHmmss_fff))
New-Item -ItemType Directory -Path $run | Out-Null
foreach ($mode in @('range-fenwick', 'range-lazy')) {
    wpr -start $profile -filemode
    if ($LASTEXITCODE) { throw 'Could not start WPR' }
    try {
        & $exe --profile $mode 100
        if ($LASTEXITCODE) { throw "Replay failed for $mode" }
    } finally {
        wpr -stop (Join-Path $run "$mode.etl")
        if ($LASTEXITCODE) { throw "Could not save the $mode trace" }
    }
}
```

Open the ETLs in Windows Performance Analyzer. Filter the CPU counter views to the benchmark executable's process and use aggregated counters for that process. Record the mode, event name, repeat count, checksum, and counter totals. Divide process totals by the number of requested operations. Report the totals as whole-process estimates because they include program startup, data generation and construction. Collect at least three paired captures, rotating which algorithm runs first. The optional Windows Performance Toolkit supplies the analyzer. See the [Microsoft PMU recording guide](https://learn.microsoft.com/en-us/windows-hardware/test/wpt/recording-pmu-events).

## Linux with perf

```bash
mkdir -p build
g++ -std=c++17 -O2 benchmarks/benchmark_range_structures.cpp -o build/benchmark_range_structures
perf stat -r 3 -e instructions:u,cycles:u,cache-misses:u \
  -- ./build/benchmark_range_structures --profile range-fenwick 100
perf stat -r 3 -e instructions:u,cycles:u,cache-misses:u \
  -- ./build/benchmark_range_structures --profile range-lazy 100
```

Check `perf list` and your system's counter permissions. Linux's generic `cache-misses` event can differ from the Windows LLC event. Preserve event names and platform when reporting measurements and compare only like events. Counter access can be unavailable in virtualized Linux environments.
