# Arena Allocator

**Version: V2 – marker-based rollback support**

## Overview

A minimal linear (bump) allocator providing fast, contiguous memory allocation over a fixed-size buffer, with checkpoint/rollback support.

- Single contiguous buffer
- Monotonic offset (no per-allocation free)
- Constant-time allocation
- Reset in O(1)
- Marker-based rollback for temporary allocations

---

## API

- `void* allocate(size, alignment)`
  - `alignment` must be a power of two and ≤ `max_alignment`
  - returns `nullptr` on failure (insufficient space or size == 0)

- `void* allocate<T>(count)`
  - allocates raw, uninitialized storage sufficient for `count` objects of type `T`
  - does not construct objects

- `void reset()`
  - resets allocation offset to zero
  - allows previously returned storage to be reused

- `Marker mark()`
  - returns a marker representing the current allocation state
  - markers are bound to the originating arena

- `void rewind(Marker marker)`
  - restores allocation state to a previously saved marker
  - storage allocated after the marker may be reused
  - requires the marker to originate from the same arena

---

## Contract

- Allocation is O(1)
- On success, returns a pointer to contiguous storage of the requested size
- Returns `nullptr` if allocation cannot be satisfied
- Returned pointers satisfy the requested alignment when `alignment ≤ max_alignment`
- `mark()` and `rewind()` allow efficient rollback of recent allocations
- Does not perform object construction or destruction

---

## Constraints & Assumptions

- Alignment must be a power of two and ≤ `max_alignment`
- The allocator does not support over-aligned types
- Memory returned is raw storage; object lifetime must be explicitly managed by the caller
- `reset()` and `rewind()` do not call destructors; any live objects must be destroyed before reuse
- Markers are only valid for the arena that created them and while that arena is alive
- Not thread-safe

---

## Tests

- Basic allocation success and failure cases
- Alignment correctness across varying alignment requests
- Exhaustion handling
- Reset behavior (full reuse)
- Marker and rewind behavior (partial rollback)
- Nested marker correctness