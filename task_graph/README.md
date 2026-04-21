# Task Graph

A minimal task graph / pipeline system built on top of the `job_system` project.

**Status:** project skeleton

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

The goal is to build a small, clear orchestration layer for dependency-driven workflows, suitable for discussion in a systems interview.

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

## Planned Public Model

The first version is intended to be batch-oriented:

1. create a graph
2. add tasks
3. add edges
4. run the graph
5. wait for completion

This keeps lifecycle and semantics simple.

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
