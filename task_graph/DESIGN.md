# Design Notes

Internal design document for the task graph system.

## Role in the stack

- Thread pool → execution engine  
- Job system → dependency scheduler  
- Task graph → workflow description and compilation layer  

The task graph converts a high-level workflow into a job-system graph.

## Core Model

- Tasks are created before execution.
- Dependencies are declared before execution.
- `run()` compiles the graph into jobs.
- `wait()` blocks until execution completes.

Execution is batch-oriented and one-shot.

## Internal Representation

Each task node contains:

- optional name
- callable
- outgoing edges

Global state:

- flat list of task nodes
- name → TaskId map (for named tasks)
- private JobSystem instance
- started flag (freezes mutation after run)

## Graph Semantics

For an edge:

A → B

- B depends on A
- B can only run after A completes

Edges are stored as adjacency lists (outgoing edges).

## Execution Model

### run()

1. validate graph is acyclic (Kahn algorithm)
2. mark graph as started
3. create one job per task
4. map TaskId → JobId
5. translate edges into job-system dependencies
6. call job system `run()`

### wait()

Delegates to the job system and blocks until completion.

## Invariants

- Each task appears exactly once in the graph.
- Task IDs refer to valid tasks.
- Named tasks are unique.
- A task cannot depend on itself.
- Duplicate edges are rejected.
- Graph is immutable after `run()`.
- `run()` may be called at most once.
- Graph must be acyclic at execution time.
- Each task is compiled to exactly one job.
- Each edge maps to one job-system dependency.
- `wait()` returns only when execution completes.

## Validation

Cycle detection uses a Kahn-style topological pass:

- compute indegree for each node
- process zero-indegree nodes
- detect incomplete traversal → cycle

Validation occurs in `run()` before compilation.

## Failure Model

- invalid API usage → exceptions
- cycles → rejected at `run()`
- job execution exceptions → handled by underlying job system

## Non-goals

- futures / return values
- cancellation
- priorities
- dynamic graph mutation during execution
- distributed execution
- persistence / serialization

## Design Trade-offs

- One-shot compilation simplifies correctness
- Delegating execution to JobSystem avoids duplication
- Explicit graph representation favors clarity
- No dynamic mutation keeps invariants simple

## Design Philosophy

- explicit over implicit  
- correctness over features  
- simple lifecycle over flexibility  
- clear semantics over abstraction complexity  
