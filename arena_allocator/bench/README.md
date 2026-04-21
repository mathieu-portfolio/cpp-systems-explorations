# Arena Benchmarks

Benchmark suite for the custom `Arena` allocator project.

## Purpose

This subproject measures the behavior of the arena allocator under controlled allocation patterns and compares it against heap-based allocation.

The goal is not to make broad allocator claims. The goal is to understand the cost model of the current design and make the main trade-offs visible:

- bump allocation cost
- reset cost after many allocations
- mark/rewind behavior
- alignment sensitivity
- fixed-capacity arena behavior versus per-allocation heap traffic

## What This Benchmark Measures

The benchmark compares two allocation strategies:

- `Arena`
- heap allocation (`::operator new` / `::operator delete`)

It currently evaluates these scenarios:

- `alloc_only`
  - perform many allocations and keep them alive until the end of the run

- `alloc_reset`
  - perform many allocations from the arena and reset once at the end of the batch
  - compare against allocating then freeing all heap blocks

- `mark_rewind`
  - repeatedly take a mark, allocate a batch, then rewind to the mark
  - compare against allocating then freeing the same batch on the heap

## Allocation Shapes

The first version supports:

- varying allocation size
- varying alignment
- varying operation count

This is enough to study how the arena behaves under common small-allocation patterns.

## Reported Metrics

For each run, the benchmark reports:

- scenario
- allocation mode
- allocation size
- alignment
- operation count
- iterations
- best time in milliseconds
- average time in milliseconds
- throughput in operations per second
- speedup relative to heap allocation

## Practical Usage

Build the benchmark:

```bash
cmake -S . -B build
cmake --build build --target arena_bench
```

Show command-line help:

```bash
./build/arena_bench --help
```

Run a single benchmark scenario:

```bash
./build/arena_bench --scenario alloc_only --size 32 --alignment 16 --count 100000 --iterations 5
```

Run a preset sweep and write CSV results:

```bash
./build/arena_bench --sweep --preset all --csv-out bench/results/results.csv
```

Generate plots from the CSV results:

```bash
python bench/plot.py --csv bench/results/results.csv --out-dir bench/results
```

### Arguments

- `--scenario` = `alloc_only`, `alloc_reset`, or `mark_rewind`
- `--size` = allocation size in bytes
- `--alignment` = allocation alignment in bytes
- `--count` = number of allocations or allocation operations
- `--iterations` = number of repeated timing runs
- `--sweep` = run a predefined benchmark matrix
- `--preset` = `sizes`, `alignments`, `patterns`, or `all`
- `--csv-out` = optional path for CSV output
- `--help` = print usage information

## Preset Sweeps

The current preset sweeps are:

- `sizes`
  - varies allocation size
  - fixed alignment
  - fixed operation count
  - uses `alloc_only`

- `alignments`
  - varies alignment
  - fixed allocation size
  - fixed operation count
  - uses `alloc_only`

- `patterns`
  - compares `alloc_only`, `alloc_reset`, and `mark_rewind`
  - fixed size, alignment, and operation count

- `all`
  - runs all preset families

## Typical Questions

This benchmark should help answer:

- How much faster is bump allocation than heap allocation for small blocks?
- How much does alignment affect arena allocation cost?
- How cheap is `reset()` compared to freeing many heap blocks?
- How useful is mark/rewind for nested temporary allocation patterns?

## Notes

- This harness is intentionally simple.
- Results are machine-specific.
- The benchmark uses a large enough arena capacity to avoid artificial out-of-memory failure during normal runs.
- Result file management is intentionally manual: choose your own output path when you want to keep a run.
