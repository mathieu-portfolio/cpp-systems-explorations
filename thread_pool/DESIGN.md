# Design Notes

Internal design document for the current thread pool implementation.

## Purpose

This file tracks the current design, invariants, behavior contracts, and near-term goals for the project.

The implementation is intentionally minimal. The main objective is to keep the concurrency model small enough to reason about precisely.

---

## Scope

### In Scope

- fixed number of worker threads
- shared queue of `std::function<void()>`
- submission from one or more producer threads
- worker coordination via mutex + condition variable
- graceful destructor-driven shutdown
- tests for basic behavior, shutdown, and concurrent submission

### Out of Scope

- futures / return values
- cancellation
- work stealing
- lock-free data structures
- dynamic resizing
- priorities
- task dependencies

---

## Current Public API

```cpp
class ThreadPool {
public:
    explicit ThreadPool(std::size_t thread_count);
    ~ThreadPool();

    void submit(std::function<void()> job);
};
```

---

## State Model

Core state:

- `workers_`
  - owns the worker threads for the entire lifetime of the pool

- `jobs_`
  - queue of submitted but not yet started jobs
  - logically owned by the pool

- `mutex_`
  - protects all shared mutable state involved in coordination

- `cv_`
  - used to wake sleeping workers when new work arrives or shutdown begins

- `stop_`
  - shutdown flag
  - indicates that no new jobs should be accepted
  - once set, workers should exit after the queue is drained

---

## Core Invariants

### Invariant 1: Shared state is synchronized consistently
All accesses to `jobs_` and `stop_` occur while holding `mutex_`.

Reason:
- the queue is not thread-safe
- worker wake/sleep decisions depend on combined state
- the condition variable predicate must observe coherent state

### Invariant 2: Queue ownership is transferred on successful submit
If `submit()` returns normally, ownership of the callable has been transferred into `jobs_`.

Reason:
- caller should no longer rely on the submitted callable object
- pool becomes responsible for eventual execution

### Invariant 3: Jobs execute outside the mutex
A worker removes one job from the queue while holding `mutex_`, then releases the lock before invoking the job.

Reason:
- prevents job execution from blocking submission and dequeue
- avoids serializing unrelated jobs under the queue lock
- reduces lock hold time

### Invariant 4: Shutdown is draining, not abandoning
Once `stop_` becomes true:
- new submissions are rejected
- already queued jobs remain eligible for execution
- workers exit only when both:
  - `stop_ == true`
  - `jobs_` is empty

### Invariant 5: No worker outlives the pool
When the destructor returns:
- all worker threads have exited
- no worker may access pool state anymore

---

## Worker Loop Contract

Worker behavior:

1. acquire `mutex_`
2. wait on `cv_` until either:
   - `stop_` is true, or
   - `jobs_` is not empty
3. if `stop_` is true and `jobs_` is empty, exit loop
4. otherwise pop one job from `jobs_`
5. release `mutex_`
6. execute the job

This gives the worker a clear exit condition and avoids busy waiting.

---

## Submit Contract

Current behavior of `submit(job)`:

- acquires `mutex_`
- checks `stop_`
- if shutdown has started, throws
- otherwise moves the job into `jobs_`
- releases `mutex_`
- notifies one worker

Important contract point:
- notification happens after enqueue
- enqueue and stop-check happen under the same mutex

---

## Shutdown Protocol

Current destructor behavior:

1. acquire `mutex_`
2. set `stop_ = true`
3. release `mutex_`
4. `notify_all()`
5. join all worker threads

Effect:
- sleeping workers wake up
- workers continue draining queued jobs
- once queue is empty, workers exit
- destructor blocks until all threads are joined

---

## Concurrency Notes

### Why a single mutex?
The queue and shutdown flag form one logical state machine:
- “is there work?”
- “should workers keep waiting?”
- “are new submissions allowed?”

Using one mutex keeps that state simple and coherent.

### Why use a condition variable?
Workers should sleep when idle instead of spinning.
The condition variable allows:
- efficient blocking when queue is empty
- wake-up on submission
- wake-up on shutdown

### Why is the wait predicate important?
Workers wait for:

```cpp
stop_ || !jobs_.empty()
```

This handles:
- normal wake-up when work arrives
- shutdown wake-up even if no more jobs will arrive
- spurious wake-ups safely

### Why execute jobs outside the lock?
Holding the queue mutex during job execution would:
- block submitters unnecessarily
- block other workers from dequeuing
- turn the queue lock into a global execution bottleneck

---

## Failure / Edge Cases

### Submission during shutdown
Current policy:
- reject by throwing from `submit()`

This makes shutdown behavior explicit, though the API may later be reconsidered.

### Job throws an exception
This is a current design pressure point.

If a job exception escapes a worker thread function, the process will terminate.
Possible policies later:
- document “jobs must not throw”
- catch all exceptions in the worker loop

This needs to be decided explicitly if the project evolves.

### Zero worker threads
This should be treated as an API decision.
Open question:
- allow it and effectively never make progress?
- reject it in the constructor?

Current recommendation: reject invalid thread counts explicitly.

---

## Testing Status

Current test coverage is intended to validate:

- submitted jobs execute
- destructor waits for queued work
- more jobs than workers are handled correctly
- concurrent submitters work correctly
- many small jobs complete

These tests do not prove full correctness, but they do exercise the intended basic contract.

---

## End Goals

- Provide a thread pool with a fixed set of worker threads and a well-defined lifecycle: threads are created at initialization, execute submitted work, and terminate only after an orderly shutdown.

- Support a minimal and explicit execution model: jobs are submitted for asynchronous execution and are eventually executed by worker threads, not by the submitting thread.

- Define a clear ownership model: submitting a job transfers responsibility for its execution to the pool; once accepted, the job will be executed exactly once.

- Guarantee safe concurrent use: multiple producers can submit jobs concurrently without data races or loss of work.

- Define deterministic shutdown semantics: once shutdown begins, no new work is accepted, all accepted work is completed, and all worker threads terminate before the pool is destroyed.

- Ensure absence of data races in the pool’s internal state through well-defined synchronization boundaries.

- Provide a clear worker contract: workers wait efficiently when no work is available, execute available jobs, and terminate only when no further work can arrive and all pending work has been processed.

- Maintain a simple and explicit state model: the system behavior can be explained in terms of a small number of states (accepting work, draining, terminated) with well-defined transitions.

- Keep the design minimal and explainable: the system should be small enough to reason about formally, with invariants and guarantees that can be stated precisely.

- Serve as a foundation for more advanced execution models, while keeping the current semantics stable and well-defined.

---

## Design Philosophy

Small, explicit, and easy to reason about.

When in doubt:
- prefer a stronger invariant
- prefer a simpler contract
- prefer clarity over extra features
