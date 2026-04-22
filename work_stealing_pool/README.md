# Work Stealing Pool

A minimal work-stealing thread pool intended as an alternative execution engine to the existing `thread_pool` project.

**Status:** baseline worker loop, per-worker local queues, and cross-worker stealing implemented

## Purpose

This project explores a different execution architecture for parallel work scheduling.

Instead of a single shared queue, each worker owns a local deque:

- workers consume local work first
- external submissions are distributed across worker queues
- idle workers attempt to steal work from other workers

## Intended Scope

### In scope

- fixed number of worker threads
- per-worker local deques
- external job submission via `submit(std::function<void()>)`
- cross-worker stealing
- clean shutdown in destructor

### Out of scope for the current step

- priorities
- futures / return values
- NUMA-aware scheduling
- task affinity
- advanced heuristics
- lock-free deque refinement

## Project Structure

- `work_stealing_pool` — library implementation
- `demo` — simple example
- `tests` — validation of basic behavior, submission, and contracts

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Notes

This is still a simple version. It uses mutex-protected deques and a linear victim scan, but it now implements actual stealing semantics.
