# Task Graph

Adds graph validation before execution.

## Behavior additions

- cycles are rejected at run()
- duplicate edges are rejected
- graph is validated before compilation to JobSystem
