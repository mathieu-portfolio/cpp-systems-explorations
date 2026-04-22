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

## First Execution Model

The first version is **scheduler-first**, not platform-first.

That means:

- define fiber states before defining context-switching internals
- define yield / resume semantics before choosing a switching mechanism
- model suspension explicitly before introducing custom stacks

This keeps the design small and debuggable.

## Core Model

- A fiber is a resumable execution context.
- Fibers are owned by the scheduler.
- Worker threads execute runnable fibers but do not own them.
- A running fiber may yield cooperatively.
- Yielding returns control to the scheduler immediately.
- A yielded fiber is suspended until resumed.
- Completion is distinct from yielding.

## Fiber States

The minimal state model for v1 is:

- `created`
- `runnable`
- `running`
- `suspended`
- `completed`

### State meanings

- `created`:
  - fiber exists but has not yet been scheduled
- `runnable`:
  - fiber is eligible to execute
- `running`:
  - fiber is currently executing on exactly one worker
- `suspended`:
  - fiber yielded cooperatively and is not currently runnable
- `completed`:
  - fiber finished execution and will not resume

## State Invariants

- A fiber is in exactly one state at a time.
- A fiber is resumed only after being created or previously suspended.
- A fiber is never executed concurrently by multiple threads.
- Yielding does not block the worker thread.
- Completed fibers do not become runnable again.
- Destroying the scheduler must not leak suspended execution contexts.

## Ownership Model

The scheduler owns all fiber objects and their lifetime.

Implications:

- submission creates scheduler-owned work
- workers borrow execution of fibers
- yielded fibers remain stored by the scheduler
- resumption re-enqueues scheduler-owned fibers into runnable work

This avoids ambiguous lifetime across worker threads.

## Yield Semantics

The defining semantic rule for v1 is:

> `yield_current()` suspends the current fiber and returns control to the scheduler. The worker thread is immediately free to run other runnable fibers.

This is the key behavior that distinguishes the system from a blocking wait model.

## First Implementation Strategy

The first implementation step should **simulate** resumable execution semantics before introducing real context switching.

That means:

- build runnable and suspended queues first
- build state transitions first
- validate scheduler ownership first
- delay platform-specific fiber switching until the model is correct

## Open Design Questions

- What platform mechanism should v1 eventually use for context switching?
- Should v1 build a standalone fiber scheduler or wrap resumable work around the existing job system?
- What is the minimal API for yield / resume?
- How should waiting on job completion interact with fiber suspension later?
- What stack allocation strategy should real fibers eventually use?

## End Goals

- Provide a minimal cooperative scheduling layer on top of the current stack.
- Make suspension and resumption explicit and explainable.
- Demonstrate how waiting can be converted from thread blocking into schedulable work.
- Preserve clarity over advanced runtime features.
