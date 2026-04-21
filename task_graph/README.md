# Task Graph

A minimal task graph / pipeline system built on top of the `job_system` project.

**Status:** graph construction, compilation to JobSystem, and batch execution implemented

## What is a task graph?

A task graph is a higher-level way to describe a workflow.

Instead of manually creating low-level jobs and dependencies, you define:

- tasks
- edges between tasks
- one full graph to execute

Example:

```text
load → parse → transform → save  
load → validate  
```

The task graph layer takes that workflow description and compiles it into dependent jobs that the job system can schedule.

## Purpose

This project is the next layer above the job system.

The job system answers:

- when a job becomes runnable
- how dependencies unlock work
- how a batch is launched and waited on

The task graph answers:

- how larger multi-stage workflows are described
- how named stages depend on each other
- how a pipeline is built from reusable steps
- how a graph of work is compiled into executable jobs

## Intended Scope

### In scope

- batch-oriented graph construction
- named tasks / stages
- explicit edges between tasks
- compilation to the underlying job system
- simple pipeline-style workflows
- full-batch execution and waiting

### Out of scope

- distributed execution
- futures / return values
- dynamic graph mutation during execution
- persistence / serialization
- advanced scheduling policies
- incremental recomputation / caching

## Usage Model

1. create a graph
2. add tasks
3. add edges
4. run the graph
5. wait for completion

## Current Behavior

- Tasks can be added with or without a name.
- Named tasks must have unique, non-empty names.
- Tasks can be retrieved by name for graph construction.
- Edges can be declared using task IDs or task names.
- A task cannot depend on itself.
- Duplicate edges are rejected.
- Task IDs are validated at API boundaries.
- The graph becomes immutable after `run()` is called.
- `run()` validates that the graph is acyclic before execution.
- `run()` compiles the task graph into a job-system batch.
- Graph edges are translated into job-system dependencies.
- `wait()` forwards to the compiled batch and blocks until completion.
- Empty graphs are allowed.

## Project Structure

- `task_graph` — library implementation (public API and orchestration logic)
- `demo` — simple example showing a small pipeline graph
- `tests` — validation of graph construction, pipeline semantics, and contracts

## Dependency

This project depends on the sibling `job_system` project and uses it as the execution layer.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Notes

This is a learning-focused implementation.

The main value of this project is not feature count, but clear semantics around graph construction, dependency flow, and execution orchestration.