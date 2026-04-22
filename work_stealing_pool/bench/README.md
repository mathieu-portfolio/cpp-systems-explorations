# Work Stealing Pool Benchmarks

Benchmark suite for the `work_stealing_pool` project.

## Purpose

This subproject measures the behavior of the work-stealing pool under controlled workloads.

The goal is not to make production-grade scheduler claims. The goal is to understand the cost of the work-stealing execution model and compare it against the lower-overhead alternatives already implemented in the project stack:

- direct single-threaded execution
- central-queue thread-pool execution
- work-stealing-pool execution

## What This Benchmark Measures

The benchmark compares three execution modes:

- `direct` → baseline, no scheduling
- `thread_pool` → central-queue execution engine
- `work_stealing_pool` → per-worker queues with stealing

These modes represent different execution strategies, not equivalent systems.
The goal is not to determine which mode is "faster", but to measure:

- the overhead introduced by each scheduler design
- how load-balancing behavior changes under different workloads

The benchmark evaluates:

- worker count
- total job count
- job granularity
- workload shape

## Expected Trends

- Small jobs:
  - Scheduling overhead dominates
  - `direct` and `thread_pool` often perform best

- Large jobs:
  - All approaches converge
  - Scheduling overhead becomes negligible relative to work

- Increasing worker count:
  - Speedup improves up to hardware limits
  - Coordination overhead limits scaling

- Uneven workloads:
  - `work_stealing_pool` should perform better than a central queue when work distribution becomes imbalanced

## Scenarios

### `uniform`

All jobs have the same amount of CPU work.

Use this to study baseline overhead and scaling under regular workloads.

### `skewed`

Most jobs are short, but some jobs are much longer.

Use this to study load-balancing behavior under uneven workloads.

### `bursty`

Jobs are submitted in bursts separated by short pauses.

Use this to study scheduler wakeup and submission overhead under intermittent load.

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
cmake --build build --target work_stealing_pool_bench
```

Show command-line help:

```bash
./build/work_stealing_pool_bench --help
```

Run a single benchmark scenario:

```bash
./build/work_stealing_pool_bench --scenario uniform --workers 4 --jobs 10000 --work 1000 --iterations 5
```

Run a preset sweep and write CSV results:

```bash
./build/work_stealing_pool_bench --sweep --preset all --csv-out bench/results/results.csv
```

Generate plots from the CSV results:

```bash
python bench/plot.py --csv bench/results/results.csv --out-dir bench/results
```

### Arguments

- `--scenario` = `uniform`, `skewed`, or `bursty`
- `--workers` = number of worker threads used by the pool
- `--jobs` = total number of jobs in the scenario
- `--work` = baseline amount of CPU work per job
- `--iterations` = number of repeated timing runs
- `--sweep` = run a predefined benchmark matrix
- `--preset` = `scaling`, `granularity`, `imbalance`, or `all`
- `--csv-out` = optional path for CSV output
- `--help` = print usage information

## Preset Sweeps

The current preset sweeps are:

- `scaling`
  - varies worker count
  - fixed uniform workload
  - compares all execution modes

- `granularity`
  - varies work per job
  - fixed uniform workload
  - compares all execution modes

- `imbalance`
  - compares `uniform`, `skewed`, and `bursty`
  - fixed worker count
  - fixed job count
  - fixed work units

- `all`
  - runs all preset families

## Typical Questions

This benchmark should help answer:

- What overhead does the work-stealing pool add over the central-queue thread pool?
- How much does stealing help under uneven workloads?
- Where does work-stealing overhead become negligible relative to useful work?
- How do the two execution engines scale as worker count increases?

## Notes

- This harness is intentionally simple.
- Results are machine-specific.
- Very small jobs exaggerate scheduling overhead.
- `skewed` uses mostly short jobs with a small fraction of long jobs.
- Result file management is intentionally manual: choose your own output path when you want to keep a run.
