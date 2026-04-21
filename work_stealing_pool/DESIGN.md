# Design Notes

Internal design document for the work-stealing pool project.

## Purpose

This document captures the intended semantics, invariants, and end goals of the system.

The work-stealing pool is an execution engine. It should compete conceptually with the existing central-queue thread pool by changing how runnable jobs are distributed across workers.

## Core Model

- Each worker owns a local queue.
- External submissions are distributed across worker queues in round-robin order.
- Workers consume local work preferentially.
- Idle workers do not steal yet in this version.
- The pool owns worker threads and joins them during shutdown.

## Current Internal Model

- `workers` stores the worker threads.
- `worker_states` stores one local deque per worker.
- each worker state has its own mutex
- a global wake mutex + condition variable is used for sleep / wakeup
- `stopping` prevents new submissions during shutdown

## Current Invariants

- A submitted job is executed at most once.
- A submitted non-empty job is eventually either executed or rejected.
- No new jobs are accepted after shutdown begins.
- A worker only pops from its own local queue in this version.
- Destructor waits for already-accepted jobs to finish before joining workers.
- Throwing jobs do not terminate worker threads.

## Execution Model

### submit()

- reject empty jobs
- reject submission after shutdown begins
- choose a target worker using round-robin selection
- push the job into that worker's local queue
- notify sleeping workers

### worker loop

- try to pop from the worker's own local queue
- if local work exists, execute it
- otherwise wait for wakeup
- on shutdown, drain any remaining local work before exiting

## Not Implemented Yet

- cross-worker stealing
- victim selection policy
- deque end asymmetry for local pop vs remote steal
- worker-local submission routing
- lock-free or low-lock deques

## End Goals

- Provide a minimal, correct work-stealing execution engine.
- Compare it against the existing central-queue thread pool under the same workloads.
- Keep the design small and explainable before exploring more advanced variants.
