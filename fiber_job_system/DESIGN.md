# Design Notes

Internal design document for the fiber job system project.

## Purpose

This document captures the intended semantics, invariants, and end goals of the system.

The fiber job system is a cooperative scheduling layer built on top of the existing execution stack. It should allow execution to suspend and resume without blocking an OS worker thread.

## Role in the stack

- Thread pool → execution engine
- Job system → dependency-aware scheduler
- Fiber job system → cooperative suspension / resumption layer

The fiber job system should convert waiting from a thread stall into schedulable work.

## First Implementation Strategy

The current version is **model-first**, not context-switch-first.

It intentionally simulates yield / resume semantics before introducing:

- custom stacks
- platform switching APIs
- saved execution context

That keeps the scheduler model small and debuggable.

## Core Model

- A fiber is a scheduler-owned resumable unit.
- Fibers are stored in runnable or suspended collections.
- Worker threads execute runnable fibers but do not own them.
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
- Destroying the scheduler must not leak runnable or suspended fibers.

## Current Yield Model

`yield_current()` works by throwing an internal scheduler exception from the currently running fiber.

This is a simulation tool, not the final mechanism.

Effects:

- current execution stops immediately
- worker thread returns to the scheduler loop
- fiber transitions to `Suspended`
- resumed fibers are later re-enqueued by `resume_all()`

## Important Limitation

The current version does **not** preserve stack or instruction position.

That means:

- resumed fibers are re-run from the beginning
- this validates scheduler state transitions
- this does not yet validate real continuation behavior

## Ownership Model

The scheduler owns all fiber objects and their lifetime.

Implications:

- submission creates scheduler-owned work
- workers borrow execution of fibers
- yielded fibers remain stored by the scheduler
- resumption re-enqueues scheduler-owned fibers into runnable work

## Execution Model

### `submit()`

- reject empty jobs
- create a new runnable fiber
- enqueue it in runnable work
- wake a worker

### worker loop

- wait for runnable work
- move one fiber into execution
- mark it `Running`
- execute its callable
- on normal completion → mark `Completed`
- on yield → mark `Suspended` and store it

### `resume_all()`

- move all suspended fibers back to runnable work
- mark them `Runnable`
- wake workers

## Open Design Questions

- What platform mechanism should the first real fiber version use for context switching?
- Should real resumption be implemented with custom stacks or a platform fiber API?
- How should waiting on job completion interact with suspension later?
- What stack allocation strategy should real fibers eventually use?

## End Goals

- Provide a minimal cooperative scheduling layer on top of the current stack.
- Make suspension and resumption explicit and explainable.
- Demonstrate how waiting can be converted from thread blocking into schedulable work.
- Preserve clarity over advanced runtime features.
