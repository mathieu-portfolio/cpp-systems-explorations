# Design Notes

Internal design document for the work-stealing pool project.

## Purpose

This document captures the intended semantics, invariants, and end goals of the system.

The work-stealing pool is an execution engine. It should compete conceptually with the existing central-queue thread pool by changing how runnable jobs are distributed across workers.

## Core Model

- Each worker owns a local queue.
- Local work is consumed preferentially by the owning worker.
- Idle workers attempt to steal work from other workers.
- The pool owns worker threads and joins them during shutdown.

## First Semantic Invariants

- A submitted job is executed at most once.
- A submitted non-empty job is eventually either executed or rejected.
- Local pop and remote steal do not both claim the same job.
- No new jobs are accepted after shutdown begins.
- Destructor waits for already-accepted jobs to finish before joining workers.

## Open Design Questions

- What deque design should v1 use?
- Should v1 use mutex-protected per-worker deques before going lower-lock?
- How should external submissions be distributed across workers?
- Should workers sleep on a global condition variable when no work is available?
- What exception policy should match the existing thread pool?

## End Goals

- Provide a minimal, correct work-stealing execution engine.
- Compare it against the existing thread pool under the same workloads.
- Keep the design small and explainable before exploring more advanced lock-free variants.
