# Arena Allocator

**Version: V1 – fixed-capacity aligned bump allocator**

## Overview

A minimal linear (bump) allocator providing fast, contiguous memory allocation over a fixed-size buffer.

- Single contiguous buffer
- Monotonic offset (no per-allocation free)
- Constant-time allocation
- Reset in O(1)

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

---

## Contract

- Allocation is O(1)
- On success, returns a pointer to contiguous storage of the requested size
- Returns `nullptr` if allocation cannot be satisfied
- Returned pointers satisfy the requested alignment when `alignment ≤ max_alignment`
- Does not perform object construction or destruction

---

## Constraints & Assumptions

- Alignment must be a power of two and ≤ `max_alignment`
- The allocator does not support over-aligned types
- Memory returned is raw storage; object lifetime must be explicitly managed by the caller
- `reset()` does not call destructors; any live objects must be destroyed before reuse
- Not thread-safe

---

## Tests

- Basic allocation success and failure cases
- Alignment correctness across varying alignment requests
- Exhaustion handling
- Reset behavior (offset reuse)