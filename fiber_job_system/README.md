# Fiber Job System

A minimal cooperative scheduling layer intended to sit above the `job_system` project.

**Status:** portable context switching implemented with `boost::context`

## What is a fiber job system?

A fiber job system allows execution to **yield and later resume at the same instruction point** without blocking an OS worker thread.

The current version uses `boost::context::continuation`:

* submitted work becomes scheduler-owned fibers
* worker threads resume runnable continuations
* `yield_current()` returns control to the worker scheduler continuation
* `resume_all()` moves suspended fibers back to runnable work
* resumed fibers continue from the same point where they yielded

## Purpose

This project explores cooperative scheduling on top of the existing execution stack.

It is intended to answer:

* how execution can suspend without blocking worker threads
* how scheduler-owned fibers are resumed later
* how portable context switching differs from platform-specific fiber APIs

## Current Model

The current implementation supports:

* `submit()` for one-shot fiber work
* `yield_current()` for cooperative suspension
* `resume_all()` to re-enqueue suspended fibers

Unlike the previous simulated model, resumed fibers continue from the exact yield point.

## Build

This project uses **vcpkg** to manage dependencies.

### Requirements

* CMake 3.21+
* C++17 compiler
* vcpkg
* `VCPKG_ROOT` environment variable set

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
ctest --preset default
```

## Notes

* Uses **Boost.Context** for portable stackful context switching.
* Not tied to Windows fibers.
* Still intentionally minimal.
* Waiting on other jobs/events is not integrated yet.
