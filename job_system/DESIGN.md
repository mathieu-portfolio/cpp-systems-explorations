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
- `run()` validates and launches the current batch.
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
- The graph must be acyclic when `run()` starts.
- A job is submitted to the thread pool only when its dependency count is zero.
- A dependent job becomes runnable exactly on the transition from `1 -> 0`.
- The unfinished-job counter is decremented exactly once per completed job.
- `wait()` returns only when the unfinished-job counter reaches zero.
- A throwing job is still treated as completed for the purposes of dependency unlocking and batch completion.

## Execution Model

### `run()`

- validates that the graph is acyclic
- marks the batch as started
- initializes the unfinished-job counter to the number of jobs in the batch
- scans the graph for jobs whose dependency count is zero
- submits those runnable jobs to the thread pool

### Cycle validation

`run()` uses a Kahn-style topological validation pass:

- copy the current dependency counts into a temporary indegree array
- enqueue every job whose indegree is zero
- repeatedly remove runnable jobs and decrement their dependents
- if the number of visited jobs is smaller than the batch size, the graph contains a cycle

If a cycle is detected, `run()` throws and the batch does not start.

### Job completion

When a job finishes:

- the callable is invoked inside a catch-all wrapper
- exceptions do not propagate out of the job system
- each dependent's dependency counter is decremented
- if a dependent transitions from `1 -> 0`, it is submitted
- the unfinished-job counter is decremented
- if the batch reaches zero unfinished jobs, `wait()` is notified

## Not Implemented Yet

- Per-job waiting
- Futures / return values
- Incremental graph mutation after execution starts
- Advanced scheduling policies
- External thread pool injection

## Open Design Questions

- What is the exact `JobId` representation long-term?
- What states should a job have internally beyond dependency count?
- What contract violations should be rejected at API boundaries?
- What graph shapes should be supported in v1 beyond acyclic dependency DAGs?

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
