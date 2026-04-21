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

## Purpose

This project builds a dependency-aware scheduling layer on top of a thread pool.

It is responsible for:

- deciding when jobs become runnable  
- tracking dependencies between jobs  
- unlocking work when jobs complete  
- executing a full batch and waiting for completion  

## Usage Model

1. create jobs  
2. add dependencies  
3. run  
4. wait  

## Behavior

- Jobs are defined with a callable.
- Dependencies are declared before execution.
- A job cannot depend on itself.
- The graph becomes immutable after `run()` is called.
- `run()` validates that the graph is acyclic.
- Runnable jobs are scheduled automatically.
- Completion unlocks dependent jobs.
- `wait()` blocks until all jobs complete.
- Empty batches are allowed.
- Job exceptions are contained and do not block progress.

## Project Structure

- `job_system` — implementation  
- `demo` — usage example  
- `tests` — validation  

## Dependency

Depends on the sibling `thread_pool` project.

## Build

cmake -S . -B build  
cmake --build build  

## Notes

Focus is on clarity, correctness, and explicit dependency semantics.
