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
- `wait()` blocks until the batch completes.

## First Semantic Invariants

- A task is represented exactly once in the graph.
- An edge adds a dependency from one task to another.
- The graph must be immutable after `run()` is called.
- `wait()` returns only after the compiled batch has completed.
- The execution graph seen by the job system must preserve the task graph’s dependency structure.

## Open Design Questions

- What is the exact `TaskId` representation?
- Should tasks be named, unnamed, or both?
- Should graph compilation be one-shot or reusable?
- How much pipeline sugar should exist on top of the graph API?
- What graph validation should happen at API boundaries versus at `run()` time?

## End Goals

- Provide a minimal task graph abstraction on top of the job system.
- Keep graph semantics explicit and easy to explain.
- Reuse the existing job system cleanly as a lower-level scheduler.
- Preserve correctness and clarity over advanced orchestration features.
