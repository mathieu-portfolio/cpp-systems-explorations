# Job System Benchmarks

Benchmark suite for the `job_system` project.

## Purpose

This subproject measures the behavior of the minimal batch-oriented job system under controlled workloads.

The goal is not to make production-grade scheduler claims. The goal is to understand the extra cost of dependency-aware scheduling and compare it against simpler execution models:

- direct single-threaded execution
- raw thread-pool execution
- job-system execution

## What This Benchmark Measures

The current benchmark compares three modes:

- `direct`
- `thread_pool`
- `job_system`

It evaluates:

- worker count
- total job count
- job granularity
- dependency shape

## Scenarios

### `independent`

All jobs are runnable immediately.

Use this to study the baseline overhead of the job system relative to the raw thread pool and direct execution.

### `chain`

Jobs form a linear dependency chain:

```text
A -> B -> C -> ...
```

Only one job is runnable at a time.

Use this to study the cost of dependency propagation and the loss of available parallelism.

### `fan_in`

Many prerequisite jobs unlock a single final dependent job:

```text
A   B   C   D
 \  |  |  /
    Final
```

Use this to study synchronization around a shared dependent.

## Reported Metrics

For each run, the benchmark reports:

- scenario
- execution mode
- worker count
- job count
- work units
- iterations
- best time in milliseconds
- average time in milliseconds
- throughput in jobs per second
- speedup versus direct execution

## Practical Usage

Build the benchmark:

```bash
cmake -S . -B build
cmake --build build --target job_system_bench
```

Show command-line help:

```bash
./build/job_system_bench --help
```

Run a single benchmark scenario:

```bash
./build/job_system_bench --scenario independent --workers 4 --jobs 10000 --work 1000 --iterations 5
```

Run a preset sweep and write CSV results:

```bash
./build/job_system_bench --sweep --preset all --csv-out bench/results/results.csv
```

Generate plots from the CSV results:

```bash
python bench/plot.py --csv bench/results/results.csv --out-dir bench/results
```

### Arguments

- `--scenario` = `independent`, `chain`, or `fan_in`
- `--workers` = number of worker threads used by the thread pool / job system
- `--jobs` = total number of jobs in the scenario
- `--work` = amount of CPU work per job
- `--iterations` = number of repeated timing runs
- `--sweep` = run a predefined benchmark matrix
- `--preset` = `scaling`, `granularity`, `dependencies`, or `all`
- `--csv-out` = optional path for CSV output
- `--help` = print usage information

## Preset Sweeps

The current preset sweeps are:

- `scaling`
  - varies worker count
  - fixed independent workload
  - compares `direct`, `thread_pool`, and `job_system`

- `granularity`
  - varies work per job
  - fixed independent workload
  - compares `direct`, `thread_pool`, and `job_system`

- `dependencies`
  - compares `independent`, `chain`, and `fan_in`
  - fixed worker count
  - fixed job count
  - fixed work units

- `all`
  - runs all preset families

## Typical Questions

This benchmark should help answer:

- What overhead does the job system add over the raw thread pool?
- How much does dependency tracking cost?
- How much parallelism is lost in a chain-shaped graph?
- How expensive is a shared fan-in unlock?
- At what job size does scheduler overhead stop dominating?

## Notes

- This harness is intentionally simple.
- Results are machine-specific.
- Very small jobs exaggerate scheduling overhead.
- The `fan_in` scenario uses `jobs - 1` prerequisite jobs and one final dependent job.
- Result file management is intentionally manual: choose your own output path when you want to keep a run.
