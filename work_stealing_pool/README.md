# Work Stealing Pool

A minimal work-stealing thread pool intended as an alternative execution engine to the existing `thread_pool` project.

**Status:** baseline worker loop and per-worker local queues implemented, stealing not implemented yet

## Purpose

This project explores a different execution architecture for parallel work scheduling.

Instead of a single shared queue, each worker owns a local queue:

- workers consume local work first
- external submissions are distributed across worker queues
- idle workers will eventually steal from other workers in a later step

## Intended Scope

### In scope

- fixed number of worker threads
- per-worker local queues
- external job submission via `submit(std::function<void()>)`
- clean shutdown in destructor

### Out of scope for the current step

- actual stealing
- priorities
- futures / return values
- NUMA-aware scheduling
- task affinity
- advanced heuristics

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

This is still an intermediate step. The current version introduces worker-owned queues and submission distribution, but does not yet perform cross-worker stealing.
