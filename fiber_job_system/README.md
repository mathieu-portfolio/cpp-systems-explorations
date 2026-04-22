# Fiber Job System

A minimal cooperative scheduling layer intended to sit above the `job_system` project.

**Status:** portable stackful context switching implemented with `boost::context`

## What is a fiber job system?

A fiber job system allows execution to **yield and later resume at the same instruction point** without blocking an OS worker thread.

The current version uses `boost::context::continuation`:

- submitted work becomes scheduler-owned fibers
- worker threads lazily start fibers on first execution
- `yield_current()` returns control to the worker scheduler continuation
- `resume_all()` moves suspended fibers back to runnable work
- resumed fibers continue from the same point where they yielded

## Purpose

This project explores cooperative scheduling on top of the existing execution stack.

It is intended to answer:

- how execution can suspend without blocking worker threads
- how scheduler-owned fibers are resumed later
- how portable stackful context switching differs from platform-specific fiber APIs

## Current Model

The current implementation supports:

- `submit()` for one-shot fiber work
- `yield_current()` for cooperative suspension
- `resume_all()` to re-enqueue suspended fibers

Unlike the previous simulated model, resumed fibers continue from the exact yield point.

## Execution Notes

This implementation creates the fiber continuation lazily on the worker thread that first executes it.

Once a fiber yields, it is resumed on that same worker. This preserves the continuation ownership model used by `boost::context` and keeps the current scheduler design simple and correct.

This means the current implementation favors correctness and clarity over free migration of yielded fibers between workers.

## Build

This project uses **vcpkg** to manage dependencies.

### Requirements

- CMake 3.21+
- C++17 compiler
- vcpkg
- `VCPKG_ROOT` environment variable set

### Setup

Install dependencies:

```bash
vcpkg install
```

### Build

Debug:

```bash
cmake --preset default
cmake --build --preset default
```

Release:

```bash
cmake --preset release
cmake --build --preset release
```

### Run tests

```bash
ctest --preset default --output-on-failure
```

## Notes

- Uses **Boost.Context** for portable stackful context switching.
- Not tied to Windows fibers.
- Yielded fibers currently keep worker affinity after first execution.
- Still intentionally minimal.
- Waiting on other jobs/events is not integrated yet.
