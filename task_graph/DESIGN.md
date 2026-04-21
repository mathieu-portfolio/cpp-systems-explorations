# Design Notes

Internal design document for the task graph project.

## Purpose

This document captures the intended semantics, invariants, and end goals of the system.

The task graph is an orchestration layer built on top of the job system. It should define how higher-level workflow stages are represented and compiled into executable dependent jobs.

## Core Model

- The thread pool is the execution engine.
- The job system is the scheduling and dependency layer.
- The task graph is the workflow-description layer.
- Tasks are created before execution begins.
- Edges are declared before execution begins.
- `run()` compiles the current graph into jobs and launches the batch.
- `wait()` blocks until the compiled batch completes.

## Current Internal Model

Each task node contains:

- an optional name
- a callable
- a list of outgoing edges

The task graph itself stores:

- a flat list of task nodes
- a private job-system instance used as the execution backend
- a started flag used to freeze graph mutation after execution begins

## Current Invariants

- A task is represented exactly once in the graph.
- An edge from `A` to `B` means `B` depends on `A`.
- A task cannot depend on itself.
- Task IDs must refer to existing tasks.
- The graph is immutable after `run()` is called.
- `run()` may be called at most once.
- Every task is compiled into exactly one job-system job.
- Every graph edge is translated into exactly one job-system dependency edge.
- `wait()` returns only after the compiled batch has completed.

## Execution Model

### `run()`

`run()` performs a one-shot compilation pass:

1. mark the graph as started
2. create one job-system job per task
3. record the `TaskId -> JobId` mapping
4. translate each graph edge into a job-system dependency
5. call `jobs_->run()`

### `wait()`

`wait()` delegates to the underlying job system and blocks until the compiled batch finishes.

## Not Implemented Yet

- Cycle validation at the task-graph layer
- Reusable compiled graphs
- Named-task lookup
- Per-task waiting
- Futures / return values
- Incremental graph mutation after execution starts
- Advanced orchestration features

## Open Design Questions

- What is the exact `TaskId` representation long-term?
- Should task names be required, optional, or externally indexed?
- Should graph compilation be one-shot or reusable?
- How much pipeline sugar should exist on top of the graph API?
- What graph validation should happen at API boundaries versus at `run()` time?

## End Goals

- Provide a minimal task graph abstraction on top of the job system.
- Keep graph semantics explicit and easy to explain.
- Reuse the existing job system cleanly as a lower-level scheduler.
- Preserve correctness and clarity over advanced orchestration features.

## Design Philosophy

Small, explicit, and easy to reason about.

When in doubt:
- prefer stronger invariants
- prefer simpler lifecycle rules
- prefer explicit graph semantics over feature growth
- prefer compilation clarity over orchestration magic
