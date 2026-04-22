# Fiber Job System Benchmarks

Benchmark suite for the `fiber_job_system` project.

## Purpose

This benchmark focuses on the one question that matters most for this project:

- what is the overhead of cooperative suspension and resumption?

Unlike the earlier scheduler benchmarks, this one is not trying to compare different pool architectures.
It is trying to measure the extra cost introduced by stackful fibers and explicit `yield_current()` / `resume_all()` behavior.

## What This Benchmark Measures

The benchmark compares three execution modes:

- `no_yield`
  - submitted fibers run to completion without yielding
  - useful as the local baseline for submit + schedule + execute cost

- `yield_once`
  - each fiber does work, yields once, then resumes and finishes
  - useful for measuring the cost of one cooperative suspension/resumption cycle

- `yield_many`
  - each fiber yields multiple times before completion
  - useful for studying how repeated yield/resume cycles scale

These modes are deliberately narrow.
The goal is to isolate fiber behavior, not to claim broad runtime performance conclusions.

## Reported Metrics

For each run, the benchmark reports:

- mode
- worker count
- fiber count
- work units per phase
- yield count
- iterations
- best time in milliseconds
- average time in milliseconds
- throughput in fibers per second
- average nanoseconds per fiber

## Preset Sweeps

The benchmark supports three preset families:

- `workers`
  - varies worker count
  - fixed work size
  - compares `no_yield` and `yield_once`

- `granularity`
  - varies work per phase
  - fixed worker count
  - compares `no_yield` and `yield_once`

- `yield_cost`
  - varies yield count per fiber
  - fixed worker count and work size
  - measures repeated suspension/resumption cost

- `all`
  - runs all preset families

## Practical Usage

Build the benchmark target:

```bash
cmake --build --preset default --target fiber_job_system_bench
```

Show command-line help:

```bash
./build/fiber_job_system_bench --help
```

Run one benchmark:

```bash
./build/fiber_job_system_bench --mode yield_once --workers 4 --fibers 10000 --work 1000 --iterations 5
```

Run a full sweep and write CSV results:

```bash
./build/fiber_job_system_bench --sweep --preset all --csv-out bench/results/results.csv
```

Generate plots from the CSV results:

```bash
python bench/plot.py --csv bench/results/results.csv --out-dir bench/results
```

## Arguments

- `--mode` = `no_yield`, `yield_once`, or `yield_many`
- `--workers` = number of worker threads
- `--fibers` = total number of submitted fibers
- `--work` = CPU work units per execution phase
- `--yields` = yields per fiber in `yield_many` mode
- `--iterations` = number of repeated timing runs
- `--sweep` = run a predefined benchmark matrix
- `--preset` = `workers`, `granularity`, `yield_cost`, or `all`
- `--csv-out` = optional path for CSV output
- `--help` = print usage information

## Interpretation Notes

- `no_yield` is the local baseline, not a separate scheduler implementation.
- `yield_once` is usually the most useful comparison because it approximates the cost of one suspend/resume cycle.
- `yield_many` is more about trend shape than raw headline numbers.
- Very small work sizes exaggerate scheduler and continuation overhead.
- Results are machine-specific.
- `resume_all()` is currently driven explicitly by the benchmark harness, which matches the current API design.

## Suggested Questions

This benchmark should help answer:

- How expensive is one yield/resume cycle relative to plain fiber execution?
- At what work size does cooperative suspension overhead become negligible?
- How does the current implementation scale with worker count for no-yield and single-yield fibers?
- How quickly does repeated yielding increase per-fiber cost?
