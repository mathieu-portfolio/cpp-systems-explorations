# Task Graph

A minimal task graph / pipeline system built on top of the `job_system` project.

**Status:** graph construction, validation, and batch execution implemented

## What is a task graph?

A task graph describes a workflow as a set of tasks and dependencies.

Instead of manually managing execution order, you define:

- tasks
- dependencies between tasks

Example:

```text
load → parse → transform → save  
load → validate  
```

Execution:
- `load` runs first
- `parse` and `validate` unlock after `load`
- `transform` runs after `parse`
- `save` runs last

The system automatically schedules and runs tasks based on these dependencies.

## Purpose

This project builds a workflow-description layer on top of the job system.

It is responsible for:

- describing multi-stage workflows
- expressing dependencies between tasks
- compiling a graph into executable jobs
- running the graph and waiting for completion

## Usage Model

1. create a graph  
2. add tasks  
3. add edges  
4. run  
5. wait  

## Behavior

- Tasks can be added with or without a name.
- Named tasks must be unique and non-empty.
- Tasks can be referenced by ID or by name.
- Edges define dependencies between tasks.
- A task cannot depend on itself.
- Duplicate edges are rejected.
- The graph becomes immutable after `run()`.
- `run()` validates that the graph is acyclic.
- The graph is compiled into a job-system batch at execution time.
- Task dependencies are enforced during execution.
- `wait()` blocks until all tasks complete.
- Empty graphs are allowed.

## Project Structure

- `task_graph` — implementation  
- `demo` — usage example  
- `tests` — validation  

## Dependency

Depends on the sibling `job_system` project.

## Build

```text
cmake -S . -B build  
cmake --build build  
```

## Notes

Focus is on clarity, correctness, and explicit workflow semantics.
