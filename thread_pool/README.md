# Thread Pool

A minimal fixed-size thread pool built as a systems programming exercise.

## Overview

This project implements a small thread pool with a fixed number of worker threads and a shared job queue. It is intentionally narrow in scope and designed to make the core concurrency mechanics explicit:

- mutex + condition variable coordination
- ownership transfer of submitted jobs
- worker thread lifecycle
- shutdown and draining semantics
- basic correctness under concurrent submission

The goal is not to compete with production libraries. The goal is to build a small, correct, well-reasoned component that is easy to discuss in a systems interview.

---

## Features

- fixed number of worker threads
- job submission via `submit(std::function<void()>)`
- internal FIFO job queue
- worker threads wait efficiently when idle
- destructor performs graceful shutdown:
  - stop accepting new work
  - finish queued work
  - join all worker threads

---

## Non-Goals

This project intentionally does not include:

- futures or return values
- work stealing
- lock-free queues
- task dependencies
- dynamic thread scaling
- advanced scheduling policies

---

## Behavior Contract

### Submission
- `submit()` transfers ownership of the callable into the pool if submission succeeds.
- If shutdown has started, `submit()` throws.

### Execution
- Jobs are executed by worker threads, not by the submitting thread.
- Jobs are removed from the queue under the mutex and executed outside the mutex.

### Shutdown
- Destruction begins shutdown by preventing further submissions.
- Already queued jobs are still executed.
- The destructor returns only after all worker threads have exited.

---

## Project Structure

```text
thread_pool/
├── CMakeLists.txt
├── README.md
├── DESIGN.md
├── include/
│   └── thread_pool.hpp
├── src/
│   └── thread_pool.cpp
├── demo/
│   └── demo.cpp
└── tests/
    ├── basic_tests.cpp
    ├── shutdown_tests.cpp
    └── stress_tests.cpp
```

---

## Build

```bash
cmake -S . -B build
cmake --build build
```

---

## Run Demo

```bash
./build/thread_pool_demo
```

On Visual Studio generators, run the produced executable from the build output directory.

---

## Run Tests

```bash
ctest --test-dir build --output-on-failure
```

---

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

---

## Why This Project Matters

A thread pool is a small but important systems component. Building one forces you to reason about:

- shared mutable state
- synchronization boundaries
- thread ownership and lifetime
- API contracts under concurrency
- shutdown semantics

These are foundational topics for larger systems such as job systems, async runtimes, servers, and game engines.

---

## Current Status

Current implementation goals:

- keep the design minimal
- prioritize correctness and clarity over features
- document invariants and shutdown behavior clearly
- use tests to validate basic, shutdown, and stress scenarios
