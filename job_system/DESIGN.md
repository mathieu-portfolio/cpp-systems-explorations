# Design Notes

Internal design document for the job system project.

## Purpose

This document captures the intended semantics, invariants, and end goals of the system.

The job system is a scheduling layer built on top of the thread pool. It should define when jobs become runnable and how job completion unlocks dependent work.

## Core Model

- The thread pool is the execution engine.
- The job system is the dependency and scheduling layer.
- Jobs are created before execution begins.
- Dependencies are declared before execution begins.
- `run()` launches the current batch.
- `wait()` blocks until the batch completes.

## Current Internal Model

Each job node contains:

- a callable
- an atomic dependency counter (`remaining_dependencies`)
- a list of dependent jobs

The job system also maintains:

- an atomic unfinished-job counter for batch completion
- a mutex and condition variable for `wait()`

Dependencies are represented as:

- the dependent job increments its dependency counter
- the prerequisite job stores the dependent in its adjacency list

## Current Invariants

- A job is scheduled for execution at most once.
- A job's `remaining_dependencies` equals the number of unmet prerequisites.
- Adding a dependency:
  - increments the dependent's counter
  - records the dependent in the prerequisite
- A job cannot depend on itself.
- Job IDs must refer to existing jobs.
- The job graph is immutable after `run()` is called.
- A job is submitted to the thread pool only when its dependency count is zero.
- A dependent job becomes runnable exactly on the transition from `1 -> 0`.
- The unfinished-job counter is decremented exactly once per completed job.
- `wait()` returns only when the unfinished-job counter reaches zero.

## Execution Model

### `run()`

- marks the batch as started
- initializes the unfinished-job counter to the number of jobs in the batch
- scans the graph for jobs whose dependency count is zero
- submits those runnable jobs to the thread pool

### Job completion

When a job finishes:

- each dependent's dependency counter is decremented
- if a dependent transitions from `1 -> 0`, it is submitted
- the unfinished-job counter is decremented
- if the batch reaches zero unfinished jobs, `wait()` is notified

## Not Implemented Yet

- Cycle detection
- Per-job waiting
- Futures / return values
- Incremental graph mutation after execution starts
- Advanced scheduling policies

## Open Design Questions

- What is the exact `JobId` representation long-term?
- What states should a job have internally beyond dependency count?
- What contract violations should be rejected at API boundaries?
- What job exception policy should the system adopt?
- What graph shapes should be supported in v1, and how should cycles be handled?

## End Goals

- Provide a minimal batch-oriented job graph executor.
- Keep dependency semantics explicit and easy to explain.
- Reuse the existing thread pool cleanly as a lower-level executor.
- Preserve correctness and clarity over advanced scheduling features.

## Design Philosophy

Small, explicit, and easy to reason about.

When in doubt:
- prefer stronger invariants
- prefer simpler lifecycle rules
- prefer explicit contracts over implicit behavior
- prefer clear scheduling semantics over feature growth
