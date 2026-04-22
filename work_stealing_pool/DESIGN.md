# Design Notes

Internal design document for the work-stealing pool project.

## Purpose

This document captures the intended semantics, invariants, and end goals of the system.

The work-stealing pool is an execution engine. It should compete conceptually with the existing central-queue thread pool by changing how runnable jobs are distributed across workers.

## Core Model

- Each worker owns a local deque.
- External submissions are distributed across worker queues in round-robin order.
- Workers consume local work preferentially.
- Idle workers attempt to steal from other workers.
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
- Local pop and remote steal do not both claim the same job.
- Owner pops from the front of its deque.
- Thieves steal from the back of victim deques.
- Destructor waits for already-accepted jobs to finish before joining workers.
- Throwing jobs do not terminate worker threads.

## Execution Model

### submit()

- reject empty jobs
- reject submission after shutdown begins
- choose a target worker using round-robin selection
- push the job into that worker's local deque
- notify sleeping workers

### worker loop

- try to pop from the worker's own local deque
- if empty, scan other workers and try to steal from the back of a victim deque
- if work exists, execute it
- otherwise wait for wakeup
- on shutdown, continue draining until no local or stealable work remains

## Current Stealing Policy

- victim selection uses a simple linear scan
- there is no randomization or affinity
- local execution and remote stealing use opposite deque ends

## Not Implemented Yet

- victim randomization
- worker-local submission routing
- lock-free or low-lock deques
- heuristics for locality or fairness
- benchmark subproject

## End Goals

- Provide a minimal, correct work-stealing execution engine.
- Compare it against the existing central-queue thread pool under the same workloads.
- Keep the design small and explainable before exploring more advanced variants.
