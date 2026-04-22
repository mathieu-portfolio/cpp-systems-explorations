# Design Notes

Internal design document for the fiber job system project.

## Purpose

This document captures the intended semantics, invariants, and end goals of the system.

The fiber job system is a cooperative scheduling layer built on top of the existing execution stack. It allows execution to suspend and resume without blocking an OS worker thread.

## Role in the stack

- Thread pool → execution engine
- Job system → dependency-aware scheduler
- Fiber job system → cooperative suspension / resumption layer

## Current Implementation Strategy

The current version uses **portable stackful continuations** via `boost::context`.

That means:

- submitted jobs are wrapped as scheduler-owned continuation-backed fibers
- worker threads resume runnable fibers
- `yield_current()` switches back to the worker scheduler continuation
- resumed work continues from the exact instruction point where it yielded

## Core Model

- A fiber is a scheduler-owned execution unit.
- Worker threads run a scheduler loop and resume runnable fibers.
- A running fiber may yield cooperatively.
- Yielding returns control to the scheduler immediately.
- Suspended fibers may later be moved back to runnable work.
- Completion is distinct from yielding.

## Fiber States

The minimal state model currently used is:

- `Runnable`
- `Running`
- `Suspended`
- `Completed`

## State Invariants

- A fiber is in exactly one state at a time.
- A fiber is never executed concurrently by multiple threads.
- Yielding does not block the worker thread.
- Completed fibers do not become runnable again.
- Suspended fibers remain scheduler-owned.
- Returning to the scheduler from a fiber always leaves the fiber in either `Suspended` or `Completed`.

## Execution Model

### `submit()`

- reject empty jobs
- create a continuation-backed fiber for the submitted callable
- enqueue runnable work
- wake a worker

### worker loop

- wait for runnable work
- move one fiber into execution
- mark it `Running`
- resume the fiber continuation
- when control returns:
  - if `Suspended` → store it
  - if `Completed` → destroy it

### `yield_current()`

- verify that the caller is the current running fiber
- mark the fiber `Suspended`
- resume the scheduler continuation

### `resume_all()`

- move all suspended fibers back to runnable work
- mark them `Runnable`
- wake workers

## Important Limitations

- Suspended fibers are resumed manually with `resume_all()`.
- Integration with job waiting / continuation semantics is not implemented yet.
- Backend portability currently depends on `boost::context` availability.

## End Goals

- Provide a minimal cooperative scheduling layer on top of the current stack.
- Make suspension and resumption explicit and explainable.
- Use real context switching rather than simulated logical steps.
- Preserve clarity over advanced runtime features.
