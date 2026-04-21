# Vector Benchmarks

Benchmark suite for the custom `Vector<T>` project.

## Purpose

This subproject measures the behavior of the custom vector under controlled workloads and compares it against `std::vector`.

The goal is not to claim production-grade performance. The goal is to understand the cost model of the current design and make the main trade-offs visible:

- growth without `reserve()`
- growth with `reserve()`
- copy cost
- move cost
- element type sensitivity

## What This Benchmark Measures

The benchmark compares two containers:

- `Vector<T>`
- `std::vector<T>`

It currently evaluates these scenarios:

- `growth`
  - push back `N` elements without reserving capacity first

- `reserve_growth`
  - reserve capacity for `N` elements, then push back `N` elements

- `copy`
  - copy a populated vector of size `N`

- `move`
  - move a populated vector of size `N`

## Element Types

The first version supports:

- `int`
- `string`
- `heavy`

The `heavy` type is an intentionally larger value type used to make copy and move costs more visible.

## Reported Metrics

For each run, the benchmark reports:

- scenario
- container mode
- element type
- size
- iterations
- best time in milliseconds
- average time in milliseconds
- throughput in operations per second
- speedup relative to `std::vector`

## Practical Usage

Build the benchmark:

```bash
cmake -S . -B build
cmake --build build --target vector_bench
```

Show command-line help:

```bash
./build/vector_bench --help
```

Run a single benchmark scenario:

```bash
./build/vector_bench --scenario growth --type int --size 100000 --iterations 5
```

Run a preset sweep and write CSV results:

```bash
./build/vector_bench --sweep --preset all --csv-out bench/results/results.csv
```

Generate plots from the CSV results:

```bash
python bench/plot.py --csv bench/results/results.csv --out-dir bench/results
```

### Arguments

- `--scenario` = `growth`, `reserve_growth`, `copy`, or `move`
- `--type` = `int`, `string`, or `heavy`
- `--size` = number of elements involved in the benchmark
- `--iterations` = number of repeated timing runs
- `--sweep` = run a predefined benchmark matrix
- `--preset` = `growth`, `copy_move`, `types`, or `all`
- `--csv-out` = optional path for CSV output
- `--help` = print usage information

## Preset Sweeps

The current preset sweeps are:

- `growth`
  - varies size
  - compares `growth` and `reserve_growth`
  - uses `int`

- `copy_move`
  - varies size
  - compares `copy` and `move`
  - uses `int`

- `types`
  - varies size
  - compares element types
  - uses `reserve_growth`

- `all`
  - runs all preset families

## Typical Questions

This benchmark should help answer:

- How much does `reserve()` reduce growth cost?
- How far is the custom vector from `std::vector` on the same workload?
- How expensive are copies compared to moves?
- How much does the element type affect performance?

## Notes

- This harness is intentionally simple.
- Results are machine-specific.
- Very small sizes may exaggerate measurement noise.
- Result file management is intentionally manual: choose your own output path when you want to keep a run.
