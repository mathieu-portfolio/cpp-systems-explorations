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

## First Semantic Invariants

- A job is scheduled for execution at most once.
- A job becomes runnable exactly when all of its dependencies are complete.
- A job is never executed before its prerequisites complete.
- Completing a job updates each dependent exactly once.
- `wait()` returns only after all jobs accepted into the current batch have completed.

## Open Design Questions

- What is the exact `JobId` representation?
- What states should a job have internally?
- What contract violations should be rejected at API boundaries?
- What job exception policy should the system adopt?
- What graph shapes should be supported in v1, and how should cycles be handled?

## End Goals

- Provide a minimal batch-oriented job graph executor.
- Keep dependency semantics explicit and easy to explain.
- Reuse the existing thread pool cleanly as a lower-level executor.
- Preserve correctness and clarity over advanced scheduling features.
