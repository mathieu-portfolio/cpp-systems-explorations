# Work Stealing Pool

A minimal work-stealing thread pool intended as an alternative execution engine to the existing `thread_pool` project.

**Status:** project skeleton

## Purpose

This project explores a different execution architecture for parallel work scheduling.

Instead of a single shared queue, each worker owns a local deque:

- workers pop from their own local queue
- idle workers attempt to steal work from other workers
- the system aims to reduce contention and improve load balance

## Intended Scope

### In scope

- fixed number of worker threads
- per-worker local queues
- stealing from other workers when idle
- job submission via `submit(std::function<void()>)`
- clean shutdown in destructor

### Out of scope

- priorities
- futures / return values
- NUMA-aware scheduling
- task affinity
- advanced heuristics
- lock-free refinement beyond the minimal design

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

This is a learning-focused implementation. The goal is to understand the mechanics and trade-offs of work stealing, not to build a production scheduler.
