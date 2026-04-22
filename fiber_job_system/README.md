# Fiber Job System

A minimal cooperative scheduling layer intended to sit above the `job_system` project.

**Status:** execution model defined, implementation still skeletal

## What is a fiber job system?

A fiber job system allows execution to **yield and later resume** without blocking an OS worker thread.

Instead of:

- a thread waiting for work to finish

the system aims for:

- the current fiber yielding
- the worker thread running other ready work
- the suspended fiber resuming later

## Purpose

This project explores cooperative scheduling on top of the existing execution stack.

It is intended to answer:

- how work can suspend without blocking worker threads
- how waiting can become a scheduling event rather than a thread stall
- how resumable execution contexts fit above the existing job system

## Current Model

The first version is intentionally scheduler-first:

- work is submitted as a resumable unit
- a running fiber may yield cooperatively
- yielding returns control to the scheduler
- yielded fibers become runnable again only when explicitly resumed
- completion is distinct from yielding

This keeps the semantics small and explicit before real context switching is introduced.

## Intended Scope

### In scope

- minimal fiber abstraction
- cooperative suspension and resumption
- explicit runnable / suspended / completed states
- integration with the existing job-system stack
- small, explainable scheduler behavior

### Out of scope

- full coroutine framework
- networking / I/O runtime
- async language integration
- preemptive scheduling
- advanced continuation chaining

## Planned Public Model

The first version keeps a small API surface:

- `submit()` to create schedulable work
- `yield_current()` to cooperatively suspend the current fiber
- internal scheduler ownership of suspended execution contexts

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

This is a learning-focused implementation. The goal is to understand cooperative scheduling and suspension semantics before introducing real context-switching machinery.
