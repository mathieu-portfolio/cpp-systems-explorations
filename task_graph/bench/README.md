# Task Graph Benchmarks

Benchmark suite for the `task_graph` project.

## Purpose

This subproject measures the behavior of the task graph layer under controlled workloads.

The goal is not to make production-grade orchestration claims. The goal is to understand the added cost of the task-graph layer and compare it against the lower layers already implemented in the project stack:

- direct single-threaded execution
- raw thread-pool execution
- job-system execution
- task-graph execution

## What This Benchmark Measures

The benchmark compares four execution modes:

- `direct` → baseline, no scheduling
- `thread_pool` → execution engine
- `job_system` → dependency scheduler
- `task_graph` → workflow description layer

These modes represent different abstraction layers, not equivalent systems.
The goal is not to determine which is "faster", but to measure:

- the overhead introduced by each layer
- how scheduling behaves under different workloads

The benchmark evaluates:

- worker count
- total task count
- task granularity
- graph shape

## Scenarios

### `independent`

All tasks are runnable immediately.

Use this to study baseline overhead at each layer.

### `chain`

Tasks form a linear dependency chain:

```text
A -> B -> C -> ...
```

Only one task is runnable at a time.

Use this to study dependency propagation overhead and loss of available parallelism.

### `fan_in`

Many prerequisite tasks unlock one final dependent task:

```text
A   B   C   D
 \  |  |  /
    Final
```

Use this to study merge-point synchronization and unlock cost.

### `fan_out`

One root task unlocks many dependent tasks:

```text
      Root
    /  |  \
   A   B   C
```

Use this to study branching overhead and parallel release of dependent work.

## Reported Metrics

For each run, the benchmark reports:

- scenario
- execution mode
- worker count
- task count
- work units
- iterations
- best time in milliseconds
- average time in milliseconds
- throughput in tasks per second
- speedup versus direct execution

## Practical Usage

Build the benchmark:

```bash
cmake -S . -B build
cmake --build build --target task_graph_bench
```

Show command-line help:

```bash
./build/task_graph_bench --help
```

Run a single benchmark scenario:

```bash
./build/task_graph_bench --scenario independent --workers 4 --tasks 10000 --work 1000 --iterations 5
```

Run a preset sweep and write CSV results:

```bash
./build/task_graph_bench --sweep --preset all --csv-out bench/results/results.csv
```

Generate plots from the CSV results:

```bash
python bench/plot.py --csv bench/results/results.csv --out-dir bench/results
```

### Arguments

- `--scenario` = `independent`, `chain`, `fan_in`, or `fan_out`
- `--workers` = number of worker threads used by the scheduler layers
- `--tasks` = total number of tasks in the scenario
- `--work` = amount of CPU work per task
- `--iterations` = number of repeated timing runs
- `--sweep` = run a predefined benchmark matrix
- `--preset` = `scaling`, `granularity`, `graphs`, or `all`
- `--csv-out` = optional path for CSV output
- `--help` = print usage information

## Preset Sweeps

The current preset sweeps are:

- `scaling`
  - varies worker count
  - fixed independent workload
  - compares all execution modes

- `granularity`
  - varies work per task
  - fixed independent workload
  - compares all execution modes

- `graphs`
  - compares `independent`, `chain`, `fan_in`, and `fan_out`
  - fixed worker count
  - fixed task count
  - fixed work units

- `all`
  - runs all preset families

## Typical Questions

This benchmark should help answer:

- What overhead does the task-graph layer add over the job system?
- How much does workflow shape change scheduling cost?
- Where does task-graph orchestration become negligible relative to work?
- How far is task-graph execution from the raw thread pool baseline?

## Expected Trends

- Small tasks:
  - Scheduling overhead dominates
  - `direct` and `thread_pool` perform best

- Large tasks:
  - All modes converge
  - Overhead becomes negligible relative to work

- Increasing worker count:
  - Speedup improves up to hardware limits
  - Higher-level abstractions scale slightly less due to coordination cost

- Graph shapes:
  - `independent`: best case for throughput
  - `chain`: worst case, no parallelism
  - `fan_in` / `fan_out`: partial parallelism with synchronization overhead

## Notes

- This harness is intentionally simple.
- Results are machine-specific.
- Very small tasks exaggerate scheduling overhead.
- Graph-based scenarios (`chain`, `fan_in`, `fan_out`) enforce dependencies.
  Lower-level modes such as `thread_pool` do not enforce these constraints,
  so comparisons should be interpreted as overhead baselines rather than equivalent execution models.
- Result file management is intentionally manual: choose your own output path when you want to keep a run.