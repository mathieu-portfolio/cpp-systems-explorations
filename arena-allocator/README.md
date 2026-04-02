# Arena Allocator

A minimal linear allocator in C++ for fast, contiguous memory allocation with alignment support.

## Overview

- Single contiguous buffer
- Bump allocation (monotonic offset)
- No per-allocation free
- Reset in O(1)

## API

- `void* allocate(size, alignment)`
  - alignment must be a power of two
  - returns nullptr on failure

- `T* allocate<T>(count)`
  - allocates raw storage for T
  - does not construct objects

- `void reset()`
  - resets allocation offset to zero

## Guarantees

- Returned pointers satisfy requested alignment
- No undefined behavior from overflow (bounds checked)
- Allocation is O(1)

## Limitations

- No deallocation (except full reset)
- Not thread-safe
- Underlying buffer alignment limits maximum supported alignment
- No object lifetime management

## Tests

- Allocation success/failure
- Alignment correctness
- Exhaustion handling
- Reset behavior