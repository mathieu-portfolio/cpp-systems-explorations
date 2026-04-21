# Design Notes

Internal design document for the job system.

## Role in the stack

- Thread pool → execution engine  
- Job system → dependency-aware scheduler  

The job system converts a dependency graph into runnable work for the thread pool.

## Core Model

- Jobs are created before execution.
- Dependencies are declared before execution.
- `run()` validates and launches a batch.
- `wait()` blocks until completion.

Execution is batch-oriented and one-shot.

## Internal Representation

Each job node contains:

- a callable
- an atomic dependency counter (remaining_dependencies)
- a list of dependents

Global state:

- unfinished_jobs_ counter
- mutex + condition variable for wait()

## Dependency Representation

For an edge A → B:

- B increments its dependency counter
- A stores B as dependent

## Execution Model

### run()

1. validate graph is acyclic (Kahn)
2. mark started
3. initialize unfinished_jobs_
4. schedule zero-dependency jobs

### completion

1. decrement dependents
2. if 1 → 0, schedule
3. decrement unfinished_jobs_
4. notify wait() if zero

## Invariants

- A job runs at most once
- Counters never go negative
- 1 → 0 transition triggers execution
- Graph immutable after run()
- Graph must be acyclic
- wait() returns when all jobs complete
- Exceptions are contained

## Failure Model

- API misuse → exceptions
- cycles → rejected at run()
- job exceptions → contained

## Non-goals

- futures
- cancellation
- priorities
- dynamic mutation
- lock-free scheduling

## Design Philosophy

- explicit over implicit
- correctness over features
- simple lifecycle over flexibility
