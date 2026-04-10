# Design Notes

---

## Purpose & Context

This project is a learning-focused implementation of arena-style allocators in C++.

The goal is to strengthen fundamentals in:

- memory layout and alignment
- pointer arithmetic and overflow safety
- ownership and object lifetime separation
- API contracts and invariants
- performance-oriented low-level design

The project is not intended to become a general-purpose allocator.  
It is designed to explore allocator behavior in a controlled and understandable way.

---

## Core Model

The allocator is based on a linear (bump) allocation model:

```text
[AAAA][BBBB][CCCC][.............]
                     ^
                  offset
```

- allocations are placed sequentially in a contiguous buffer
- the offset always moves forward
- no individual deallocation exists

Allocation is implemented as:

1. compute aligned address from current offset
2. return pointer
3. advance offset

---

## Alignment Strategy

The arena buffer is allocated with a fixed maximum alignment:

- all requested alignments must be non-zero powers of two
- all requested alignments must satisfy: `alignment <= max_alignment`
- alignment is enforced via padding

```text
Before alignment:

[AAAA][BBBB][x][........]
             ^
          misaligned

After padding:

[AAAA][BBBB][pad][CCCC][....]
                   ^
                aligned
```

This lets the allocator satisfy supported alignments without requiring per-allocation system calls.

---

## Marker / Rewind Model

Markers allow restoring a previous allocation state:

```text
mark()

[AAAA][BBBB][.............]
               ^
            marker

allocate more:

[AAAA][BBBB][CCCC][DDDD][..]
                         ^
                      offset

rewind(marker):

[AAAA][BBBB][.............]
               ^
            offset restored
```

Properties:

- `mark()` captures the current offset
- `rewind()` restores the offset
- `reset()` invalidates previously created markers
- markers are only valid for the arena instance that created them
- a scoped rewind guard captures a marker and rewinds automatically on destruction
- `dismiss()` disables the automatic rewind
- memory is not cleared, only made reusable
- no destructors are called

---

## Invariants

The allocator relies on the following invariants:

- `0 <= offset <= capacity`
- all returned pointers lie within `[buffer, buffer + capacity)`
- offset only moves forward, except through `rewind()` or `reset()`
- allocation succeeds only if both alignment padding and requested size fit within remaining capacity
- an active scoped rewind guard owns a valid rewind point for the current arena state

Correctness of allocation depends on maintaining these invariants.

---

## Error Model

The allocator distinguishes between:

### Runtime failure

- insufficient capacity -> returns `nullptr`

### Programmer error

- invalid alignment
- alignment greater than `max_alignment`
- invalid marker usage
- invalidation of an active scoped rewind guard

These are contract violations:

- checked explicitly in both debug and release builds
- reported with file/line information
- treated as fail-fast errors

This keeps the allocator minimal and fast.

---

## Lifetime Model

The arena provides raw storage only:

- it does not construct objects
- it does not destroy objects

This keeps allocation separate from object lifetime management.

Important implications:

> `rewind()` and `reset()` invalidate storage without calling destructors.

> An active scoped rewind guard also participates in lifetime management of arena state: destroying it rewinds storage unless it has been dismissed.

---

## Design Trade-offs

### Advantages

- O(1) allocation
- O(1) reset / rewind
- no fragmentation
- good cache locality
- simple mental model

### Limitations

- no individual deallocation
- requires grouped lifetimes
- not thread-safe
- not suitable for automatic destruction of non-trivial objects

---

## Current Design Direction

The implementation intentionally favors:

- simplicity over generality
- explicit contracts over defensive runtime checks
- raw memory control over abstraction

It is designed as a scratch allocator for:

- request-scoped memory
- temporary computation buffers
- short-lived working data

---

## Target Feature Set

Possible extensions include:

- support for externally provided buffers
- limited typed construction helpers

New features should only be added when they deepen understanding of memory behavior, lifetime management, or allocator trade-offs.

---

## Non-Goals

To preserve clarity, the project intentionally avoids:

- general-purpose allocator replacement
- automatic destructor tracking
- thread safety mechanisms
- advanced allocation strategies such as pool or freelist allocators

---

## Design Philosophy

> A simple model that is easy to reason about is more valuable than a feature-rich one.

The goal is not to hide complexity, but to expose it clearly.
