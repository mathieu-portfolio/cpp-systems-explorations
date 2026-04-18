# Design Notes

Internal design document for the thread pool project.

## Purpose

This document captures the intended semantics, invariants, and end goals of the system. It is meant to define what the pool must guarantee, not just describe how the current code happens to work.

---

## Scope

### In Scope

- fixed number of worker threads
- asynchronous submission of fire-and-forget jobs
- concurrent submission from multiple producers
- orderly draining shutdown
- explicit API contracts and invariants
- tests for basic behavior, shutdown behavior, contract enforcement, and concurrent submission

### Out of Scope

- futures / return values
- cancellation
- work stealing
- lock-free data structures
- dynamic resizing
- priorities
- task dependencies

---

## Core Semantics

- The pool owns a fixed set of worker threads for its entire lifetime.
- Submitted jobs are executed asynchronously by worker threads.
- Successfully accepted jobs are the responsibility of the pool until completion.
- Shutdown is draining, not abandoning: accepted work completes before the pool is destroyed.
- The pool exposes a small contract surface and favors explicit failure over ambiguous behavior.

---

## Semantic Invariants

- Accepted work is never lost: once a job is accepted by the pool, it remains the pool's responsibility until completion.

- Accepted work executes at most once: no accepted job may be started or completed more than once.

- No work is accepted after shutdown begins: once the pool enters shutdown, all subsequent submissions are rejected.

- Workers do not terminate while accepted work remains incomplete: shutdown is draining, not abandoning.

- Destruction is a full lifetime barrier: when the pool is destroyed, all accepted work has completed and no worker may still access pool-owned state.

- Pool behavior is determined only by synchronized shared state: decisions about accepting work, waiting, executing, and terminating are based on coherent state transitions.

- Idle workers wait efficiently for meaningful state changes: workers block when no work is available and wake only when work arrives or shutdown progresses.

---

## End Goals

- Provide a thread pool with a fixed set of worker threads and a well-defined lifecycle: threads are created at initialization, execute submitted work, and terminate only after an orderly shutdown.

- Support a minimal and explicit execution model: jobs are submitted for asynchronous execution and are eventually executed by worker threads, not by the submitting thread.

- Define a clear ownership model: submitting a job transfers responsibility for its execution to the pool; once accepted, the job will be executed exactly once.

- Guarantee safe concurrent use: multiple producers can submit jobs concurrently without data races or loss of work.

- Define deterministic shutdown semantics: once shutdown begins, no new work is accepted, all accepted work is completed, and all worker threads terminate before the pool is destroyed.

- Ensure absence of data races in the pool's internal state through well-defined synchronization boundaries.

- Provide a clear worker contract: workers wait efficiently when no work is available, execute available jobs, and terminate only when no further work can arrive and all pending work has been processed.

- Maintain a simple and explicit state model: the system behavior can be explained in terms of a small number of states with well-defined transitions.

- Keep the design minimal and explainable: the system should be small enough to reason about formally, with invariants and guarantees that can be stated precisely.

- Serve as a foundation for more advanced execution models, while keeping the current semantics stable and well-defined.

---

## Public Contract

### Construction

- Constructing the pool with zero worker threads is invalid and throws.
- Successful construction creates the pool's full worker set.

### Submission

- Submitting a valid job either succeeds and transfers responsibility to the pool, or fails without the job becoming accepted.
- Submitting an empty job is invalid and throws.
- Submitting after shutdown begins is rejected and throws.

### Shutdown

- Shutdown begins when the pool stops accepting new work.
- Once shutdown begins, previously accepted jobs remain executable.
- Destruction returns only after accepted work has finished and workers have terminated.

---

## Runtime Checks vs Semantic Guarantees

### Checked directly at runtime

- constructor preconditions such as a non-zero worker count
- submission preconditions such as a non-empty callable
- internal contract violations that represent impossible or invalid states for the implementation

### Guaranteed by design and structure

- accepted work is not abandoned during shutdown
- accepted work is not executed more than once
- workers do not outlive the pool
- worker progress and termination are driven by synchronized state transitions

### Validated through tests

- basic execution of submitted jobs
- draining shutdown behavior
- behavior under multiple producers
- stress behavior with many small jobs
- contract rejection for invalid construction and invalid submission

---

## Open Questions

- Exception policy for jobs: whether job exceptions are forbidden by contract or must be caught internally.
- Exact long-term state model: whether a single shutdown flag remains sufficient or whether explicit named states become clearer.
- Whether additional externally visible shutdown operations are useful, or whether destructor-driven shutdown should remain the only mechanism.

---

## Design Philosophy

Small, explicit, and easy to reason about.

When in doubt:
- prefer a stronger semantic guarantee
- prefer a simpler contract
- prefer explicit rejection over ambiguous behavior
- prefer correctness and clarity over additional features
