# C++ Systems Explorations

A collection of small projects to explore low-level C++ concepts.

## Approach

Each project in this repository focuses on a single concept:

- minimal surface area
- explicit ownership and lifetime
- clear invariants and contracts

The goal is to build small systems that are easy to reason about and discuss.

## Projects

### Memory and Containers

- [Arena Allocator](./arena_allocator/): linear allocator with explicit lifetime and no per-object deallocation
- [Custom Vector](./custom_vector/): contiguous dynamic array with manual memory management

### Concurrency and Scheduling

- [Thread Pool](./thread_pool/): fixed-size worker pool with a central task queue
- [Job System](./job_system/): lightweight task system with dependency counting
- [Task Graph](./task_graph/): DAG-based scheduling with explicit task dependencies
- [Work Stealing Pool](./work_stealing_pool/): per-worker queues with work stealing for load balancing
- [Fiber Job System](./fiber_job_system/): cooperative scheduling with stackful fibers and explicit yield/resume