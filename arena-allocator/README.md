# Arena Allocator

**Version: V2 – marker-based rollback support**

---

## Problem

Many systems need to create large amounts of **temporary data**:

- parsing input
- processing requests
- building intermediate results

This data is often:
- dynamically allocated
- short-lived
- sharing the same lifetime

Using general-purpose allocation (`new`, `malloc`, STL containers) can lead to:

- many small allocations
- overhead from allocation/deallocation
- fragmented memory
- more complex ownership and lifetime management

---

## Solution

An **arena allocator** groups allocations together and frees them in one step.

Instead of managing each allocation individually:

- allocate memory linearly from a buffer
- do not free individual allocations
- reclaim memory all at once with `reset()` or `rewind()`

> **Allocate many things, free them all at once.**

---

## How It Works

### Linear allocation (bump pointer)

```text
Initial state:

[..............................................]
^
buffer start

After allocations:

[AAAA][BBBBBB][CC][DDDDD][.................]
                              ^
                           offset (next free)
```

- allocations are contiguous
- allocation only moves the offset forward
- no per-allocation free
- allocation is O(1)

### Allocation

```text
Before:

[AAAA][BBBB][.................]
               ^
            offset

Allocate 6 bytes:

[AAAA][BBBB][CCCCCC][.........]
                     ^
                  offset moved forward
```

### Mark / Rewind (V2 feature)

```text
mark()

[AAAA][BBBB][.................]
               ^
             marker

allocate more:

[AAAA][BBBB][CCCC][DDDD][.....]
                         ^
                      offset

rewind(marker):

[AAAA][BBBB][.................]
               ^
            offset restored
```

- everything after the marker is reclaimed instantly
- no per-allocation free is required

---

## Real Usage Pattern

Arena allocators are useful when **many allocations share the same lifetime**.

A common example is request-scoped scratch memory:

```text
Request start:

[..............................................]
^ offset = 0

During processing:

[Normalized][Tokens][TempData][...............]
                               ^
                            offset

After rewind:

[..............................................]
^ offset = 0
```

Typical pattern:

- allocate temporary working data during a task
- use that data to compute a result
- reclaim all temporary memory in one step

---

## Where This Is Used

Arena-style allocation is widely used in:

- game engines (per-frame allocations)
- compilers and parsers (AST and intermediate structures)
- high-performance servers (per-request scratch memory)
- serialization systems
- embedded systems

These systems all share the same pattern:

> Many allocations are created and discarded together.

---

## This Project

This implementation provides:

- fixed-size contiguous buffer
- aligned allocation
- constant-time reset
- marker-based rollback
- raw storage only (no object construction or destruction)
- explicit contract enforcement in debug and release builds
- single-threaded design

It is designed as a **scratch allocator** for temporary working data.

---

## API Contract

- `alignment` must be non-zero
- `alignment` must be a power of two
- `alignment` must not exceed `max_alignment`
- markers are only valid for the arena that created them
- markers may become invalid after `reset()` or after rewinding past them
- allocation failure due to insufficient capacity returns `nullptr`
- returned memory is raw, uninitialized storage

---

## Example Usage

```cpp
Arena arena(...);

auto marker = arena.mark();

// allocate temporary data
void* buffer = arena.allocate(...);

// process data

arena.rewind(marker); // reclaim temporary allocations
```

---

## Project Structure

- `arena` (library): allocator implementation
- `arena_tests`: correctness and invariants
- `arena_demo`: practical usage example

---

## Notes

- Memory returned is raw storage; object lifetime is managed by the caller
- `reset()` and `rewind()` do not call destructors
- Invalid usage is treated as a programmer error; contract violations are reported with file/line information and terminate execution
- Not thread-safe

---

For more details on design decisions, implementation direction, and project goals, see `DESIGN.md`.
