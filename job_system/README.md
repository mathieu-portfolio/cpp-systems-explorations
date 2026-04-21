# Job System

A minimal batch-oriented job system built on top of the `thread_pool` project.

**Status:** dependency graph, execution path, cycle rejection, and exception containment implemented

## What is a job system?

A job system turns a **graph of work** into **correct parallel execution**.

- A thread pool runs tasks  
- A job system decides **when** tasks are allowed to run  

Example:

A → B → C  
A → D  

Execution:
- A runs first  
- B and D unlock and run in parallel  
- C runs after B  

You describe dependencies. The system handles ordering and parallelism.

---

## Purpose

This project is the next layer above the thread pool.

The thread pool answers:

- how worker threads execute runnable jobs
- how work is queued
- how shutdown is coordinated

The job system answers:

- when a job becomes runnable
- how dependencies between jobs are represented
- how completion of one job unlocks another
- how a batch of work is launched and waited on

---

## Intended Scope

### In scope

- batch-oriented execution model
- jobs identified through lightweight handles
- explicit dependency edges between jobs
- runnable jobs submitted to the thread pool
- completion tracking
- `wait()` for a full batch
- cycle rejection at execution start
- contained job exceptions

### Out of scope

- futures / return values
- cancellation
- priorities
- work stealing
- dynamic graph mutation during execution
- lock-free scheduling
- persistence / serialization

---

## Usage Model

1. create jobs  
2. add dependencies  
3. run  
4. wait  

---

## Current Behavior

- jobs are defined with a callable
- dependencies are declared before execution
- the graph becomes immutable at `run()`
- `run()` validates that the graph is acyclic
- runnable jobs are submitted automatically
- completion unlocks dependents
- `wait()` blocks until all jobs complete
- empty batches are allowed
- job exceptions are caught and do not block progress

---

## Project Structure

- `job_system` — implementation
- `demo` — example
- `tests` — validation

---

## Dependency

Depends on the sibling `thread_pool` project.

---

## Build

```bash
cmake -S . -B build
cmake --build build
```

---

## Notes

Focus is on clarity and correctness.
