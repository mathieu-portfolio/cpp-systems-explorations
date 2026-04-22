# Design Notes

Internal design document for the fiber job system project.

## Purpose

This document captures the intended semantics, invariants, and end goals of the system.

The fiber job system is a cooperative scheduling layer built on top of the existing execution stack. It should allow execution to suspend and resume without blocking an OS worker thread.

## Role in the stack

- Thread pool → execution engine
- Job system → dependency-aware scheduler
- Fiber job system → cooperative suspension / resumption layer

## Current Implementation Strategy

The current version is still model-first, but now moves resumable state ownership into dedicated task objects.

That means:

- resume state belongs to the fiber task
- scheduler owns the fiber and its task lifetime
- lambda-based resumable submission is now only a convenience adapter

This is one step closer to real fibers, where execution state belongs to the resumable unit itself.

## Core Model

- A fiber is a scheduler-owned execution unit.
- A resumable fiber owns a `FiberTask`.
- The task object stores logical progress across resumes.
- Worker threads execute runnable fibers but do not own them.
- Suspended fibers may later be moved back to runnable work.
- Completion is distinct from yielding.

## Fiber Kinds

### One-shot fibers

- hold `std::function<void()>`
- execute once
- may still suspend via `yield_current()`
- restart from the beginning if resumed after `yield_current()`

### Task-based resumable fibers

- hold `std::unique_ptr<FiberTask>`
- call `FiberTask::run()` each time they are scheduled
- preserve logical progress inside task fields
- return `Yield` to suspend
- return `Complete` to finish

## State Invariants

- A fiber is in exactly one state at a time.
- A fiber is never executed concurrently by multiple threads.
- Yielding does not block the worker thread.
- Completed fibers do not become runnable again.
- Suspended fibers remain scheduler-owned.
- A `FiberTask` is owned by exactly one fiber.

## Execution Model

### `submit_task()`

- reject null tasks
- create a new runnable fiber
- transfer ownership of the task into the fiber
- enqueue runnable work
- wake a worker

### worker loop

- wait for runnable work
- move one fiber into execution
- mark it `Running`
- execute one-shot or task-based behavior
- on normal completion → mark `Completed`
- on yield → mark `Suspended` and store it

### `resume_all()`

- move all suspended fibers back to runnable work
- mark them `Runnable`
- wake workers

## Important Limitation

The current version still does not preserve native stack or instruction position.

That means:

- `FiberTask` preserves logical state, not CPU execution context
- one-shot jobs that yield still restart from the beginning
- true continuation semantics still require real context switching later

## End Goals

- Provide a minimal cooperative scheduling layer on top of the current stack.
- Make suspension and resumption explicit and explainable.
- Move toward true per-fiber execution ownership before real context switching.
- Preserve clarity over advanced runtime features.
