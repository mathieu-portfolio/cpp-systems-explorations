# Fiber Job System

A minimal cooperative scheduling layer intended to sit above the `job_system` project.

**Status:** simulated scheduler with runnable/suspended states implemented, real context switching not implemented

## What is a fiber job system?

A fiber job system allows execution to yield and later resume without blocking an OS worker thread.

The current version simulates that model:

- submitted work becomes scheduler-owned runnable fibers
- a running fiber may call `yield_current()`
- yielding suspends the current fiber
- `resume_all()` moves suspended fibers back to runnable work

This version does not yet preserve stack context. Resumed work is re-executed from the start.

## Purpose

This project explores cooperative scheduling semantics on top of the existing execution stack before introducing real context-switching machinery.

It is intended to answer:

- how runnable and suspended work are represented
- how yielding changes scheduler state
- how worker threads remain free to run other work

## Current Model

The first implementation is intentionally simulated:

- fibers have explicit scheduler-managed states
- worker threads execute runnable fibers
- yielding suspends the current fiber by throwing an internal scheduler exception
- suspended fibers may later be re-enqueued with `resume_all()`
- completion is distinct from yielding

## Intended Scope

### In scope

- minimal fiber abstraction
- cooperative suspension and resumption
- explicit runnable / suspended / completed states
- small, explainable scheduler behavior

### Out of scope for the current step

- real stack preservation
- platform-specific context switching
- async language integration
- preemptive scheduling
- advanced continuation chaining

## Planned Public Model

The current API surface is:

- `submit()` to create schedulable work
- `yield_current()` to cooperatively suspend the current fiber
- `resume_all()` to move suspended fibers back to runnable work

## Project Structure

- `fiber_job_system` — library implementation
- `demo` — simple example
- `tests` — validation of scheduler contracts and state semantics

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Notes

This is a learning-focused implementation. The goal is to validate scheduler ownership and yield semantics before introducing real fiber context switching.
