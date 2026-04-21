# Thread Pool Benchmarks

Benchmark suite for the `thread_pool` project.

## Purpose

This subproject measures the behavior of the minimal thread pool under controlled workloads.

The goal is not to produce production-grade performance claims. The goal is to understand the cost model of the current design and evaluate how it behaves as workload shape and contention change.

---

## What This Benchmark Measures

The current benchmark harness compares:

- direct single-threaded execution
- execution through the thread pool

It is designed to evaluate the impact of:

- worker thread count
- number of jobs
- number of concurrent submitter threads
- job granularity
- workload type

---

## Initial Workloads

### Spin

Synthetic CPU-bound work implemented as a deterministic arithmetic loop.

Use this to study:

- scheduling overhead
- scaling with worker count
- impact of job granularity
- submission contention

### Sleep

Artificial blocking work implemented with `sleep_for`.

Use this only as an illustration of overlap and shutdown behavior.
It is less useful than CPU-bound work for real performance analysis.

---

## Reported Metrics

For each run, the benchmark reports:

- scenario name
- workload type
- worker count
- producer count
- job count
- work units
- total wall-clock time in milliseconds
- throughput in jobs per second
- speedup versus direct execution

---

## Practical Usage

Build the benchmark:

```bash
cmake -S . -B build
cmake --build build --target thread_pool_bench
```

Show command-line help:

```bash
./build/thread_pool_bench --help
```

Run a single benchmark scenario:

```bash
./build/thread_pool_bench --workers 4 --jobs 10000 --producers 1 --workload spin --work 1000 --iterations 5
```

Run a preset sweep and write CSV results:

```bash
./build/thread_pool_bench --sweep --preset all --csv-out bench/results/results.csv
```

Generate plots from the CSV results:

```bash
python bench/plot.py --csv bench/results/results.csv --out-dir bench/results
```

### Arguments

- `--workers` = number of worker threads used by the pool
- `--jobs` = total number of submitted jobs
- `--producers` = number of threads concurrently calling `submit()`
- `--workload` = `spin` or `sleep`
- `--work` = amount of work per job
- `--iterations` = number of repeated timing runs
- `--sweep` = run a predefined benchmark matrix
- `--preset` = `scaling`, `granularity`, `contention`, or `all`
- `--csv-out` = optional path for CSV output
- `--help` = print usage information

---

## Preset Sweeps

The current preset sweeps are:

- `scaling`
  - varies worker count
  - fixed job count
  - fixed producer count
  - fixed spin workload sizes

- `granularity`
  - varies work per job
  - fixed worker count
  - fixed producer count
  - fixed job count

- `contention`
  - varies producer count
  - fixed worker count
  - fixed job count
  - fixed spin workload size

- `all`
  - runs all preset families

---

## Typical Questions

This benchmark should help answer:

- How does throughput change as worker count increases?
- At what job size does the pool become worthwhile?
- How much multi-producer contention hurts submission throughput?
- When direct execution is faster than pooled execution?

---

## Notes

- This harness is intentionally simple.
- Results are machine-specific.
- Very small jobs will exaggerate queueing and synchronization overhead.
- Sleep-based workloads are easier to visualize but less useful than CPU-bound workloads for serious analysis.
- Result file management is intentionally manual: choose your own output path when you want to keep a run.
