# Job System

A minimal batch-oriented job system built on top of the `thread_pool` project.

**Status:** project skeleton

## Purpose

This project is the next layer above the thread pool.

The thread pool answers:

- how worker threads execute runnable jobs
- how work is queued
- how shutdown is coordinated

The job system answers:

- when a job becomes runnable
- how dependencies between jobs are represented
- how completion of one job unlocks another
- how a batch of work is launched and waited on

The goal is to build a small, well-reasoned scheduler with explicit dependency semantics, suitable for discussion in a systems interview.

## Intended Scope

### In scope

- batch-oriented execution model
- jobs identified through lightweight handles
- explicit dependency edges between jobs
- runnable jobs submitted to the thread pool
- completion tracking
- `wait()` for a full batch

### Out of scope

- futures / return values
- cancellation
- priorities
- work stealing
- dynamic graph mutation during execution
- lock-free scheduling
- persistence / serialization

## Planned Public Model

The first version is intended to be batch-oriented:

1. create jobs
2. add dependencies
3. run the batch
4. wait for completion

This keeps the lifecycle simple and makes the semantics easier to reason about.

## Project Structure

- `job_system` — library implementation (public API and scheduling logic)
- `demo` — simple example showing how to build and run a small job graph
- `tests` — validation of basic behavior, dependencies, waiting, and contracts

## Dependency

This project depends on the sibling `thread_pool` project and uses it as the execution layer.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Notes

This is a learning-focused implementation.

The main value of this project is not feature count, but clear semantics around dependency tracking, scheduling, and completion.
