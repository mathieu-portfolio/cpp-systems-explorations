# Thread Pool

**Version:** v1.0

A minimal fixed-size thread pool built as a systems programming exercise.

## Overview

This project implements a small thread pool with a fixed number of worker threads and a shared job queue. It is intentionally narrow in scope and designed to make the core concurrency semantics explicit:

- asynchronous job submission
- worker thread lifecycle
- ownership transfer of submitted jobs
- graceful draining shutdown
- basic correctness under concurrent submission

The goal is not to compete with production libraries. The goal is to build a small, correct, well-reasoned component that is easy to discuss in a systems interview.

## Features

- fixed number of worker threads
- job submission via `submit(std::function<void()>)`
- internal FIFO job queue
- worker threads wait efficiently when idle
- destructor performs graceful shutdown:
  - stop accepting new work
  - finish queued work
  - join all worker threads

## Contract

- `ThreadPool(thread_count)` requires `thread_count > 0`; constructing with zero threads throws.
- `submit(job)` requires a valid callable; submitting an empty job throws.
- Once shutdown begins, new submissions are rejected and throw.
- Once a job is accepted, it becomes the pool's responsibility for execution.
- The destructor returns only after all accepted work has finished and all worker threads have terminated.

## Non-Goals

This project intentionally does not include:

- futures or return values
- work stealing
- lock-free queues
- task dependencies
- dynamic thread scaling
- advanced scheduling policies

## Project Structure

- `thread_pool` — library implementation (public API and core logic)
- `demo` — simple example showing how to use the thread pool
- `tests` — validation of behavior, shutdown semantics, concurrency, and contracts

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run Demo

```bash
./build/thread_pool_demo
```

On Visual Studio generators, run the produced executable from the build output directory.

## Run Tests

```bash
ctest --test-dir build --output-on-failure
```

## Example

```cpp
ThreadPool pool(4);

pool.submit([] {
    // do work
});

pool.submit([] {
    // do more work
});
```

## Why This Project Matters

A thread pool is a small but important systems component. Building one forces you to reason about:

- shared mutable state
- synchronization boundaries
- thread ownership and lifetime
- API contracts under concurrency
- shutdown semantics

These are foundational topics for larger systems such as job systems, async runtimes, servers, and game engines.
