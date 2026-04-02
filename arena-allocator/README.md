# Arena Allocator

## Overview

A minimal linear (bump) allocator providing fast, contiguous memory allocation over a fixed-size buffer.

- Single contiguous buffer
- Monotonic offset (no per-allocation free)
- Constant-time allocation
- Reset in O(1)

---

## API

- `void* allocate(size, alignment)`
  - `alignment` must be a power of two
  - returns `nullptr` on failure (invalid alignment or insufficient space)

- `T* allocate<T>(count)`
  - allocates raw, uninitialized storage for `T`
  - does not construct objects (caller must use placement new if needed)

- `void reset()`
  - resets allocation offset to zero
  - invalidates all previously returned pointers

---

## Contract

- Allocation is O(1)
- On success, returns a pointer to contiguous storage of the requested size
- Returns `nullptr` if allocation cannot be satisfied
- Returned pointers satisfy the requested alignment, provided the alignment is supported by the underlying storage
- Does not perform object construction or destruction

---

## Constraints & Assumptions

- Alignment must be a power of two
- The allocator does not guarantee support for over-aligned types (alignment greater than that of the backing buffer)
- Memory returned is raw storage; object lifetime must be explicitly managed by the caller
- `reset()` does not call destructors; any live objects must be destroyed before reset
- Not thread-safe

---

## Tests

- Basic allocation success and failure cases
- Alignment correctness across varying alignment requests
- Exhaustion handling
- Reset behavior (offset reuse)