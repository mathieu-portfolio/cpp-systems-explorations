# Design Notes

## Purpose

This project is a learning-focused implementation of a vector-like dynamic array in C++.

The goal is not to rebuild the full standard library, but to understand the design of a small container that owns contiguous storage and manages object lifetime explicitly.

It is meant to strengthen fundamentals in:

- memory ownership
- object lifetime
- invariants and API contracts
- growth and reallocation strategy
- move vs copy behavior
- performance reasoning

---

## Design Scope

The design intentionally stays narrow.

### In scope

- contiguous storage
- `push_back` / `emplace_back`
- `reserve` / `capacity` / `size`
- `operator[]`
- `clear` / destructor
- basic copy and move semantics

### Out of scope

- allocators
- full STL compatibility
- advanced iterator model
- strong exception guarantees
- thread safety

The container is designed to be small, correct, and explainable.

---

## Core Model

The vector owns a contiguous memory buffer and tracks two different things:

- **capacity**: how much storage is available
- **size**: how many objects are currently alive

This leads to the central design idea:

> Allocated storage is not the same thing as constructed objects.

The buffer may have room for many elements, while only a prefix of that buffer actually contains live objects.

This separation is the foundation of the whole design.

---

## Ownership Model

The vector has exclusive ownership of its storage.

That ownership includes two responsibilities:

1. acquiring and releasing the raw memory buffer
2. constructing and destroying the objects stored inside it

This makes the container responsible not only for memory, but for lifetime transitions.

In practical terms:

- construction creates storage or an empty state
- insertion creates new objects in owned storage
- `clear()` destroys objects but keeps storage
- destruction destroys objects and then releases storage
- move operations transfer ownership and leave the source valid

---

## Invariants

The design relies on a small set of invariants:

- `0 <= size_ <= capacity_`
- `data_ == nullptr` iff `capacity_ == 0`
- elements in `[0, size_)` are constructed
- slots in `[size_, capacity_)` are storage only

These invariants keep the model simple and make each operation easier to reason about.

---

## Reallocation Strategy

When the vector runs out of space, it allocates a larger buffer and relocates existing elements.

The high-level process is:

1. allocate new storage
2. construct relocated elements in the new buffer
3. destroy the old elements
4. release the old storage
5. update the internal state

This preserves logical content while replacing the physical buffer.

A geometric growth strategy is used:

- first growth from zero goes to `1`
- later growth doubles the capacity

This keeps repeated append operations amortized constant time.

---

## Copy and Move Design

Copy and move are treated as distinct design problems.

### Copy

Copying creates independent ownership.

A copied vector receives its own storage and its own constructed elements.  
The source remains unchanged.

### Move

Moving transfers ownership.

A moved-to vector takes the source buffer directly, avoiding per-element copying.  
The moved-from vector is reset to a valid empty state.

This distinction is central to both correctness and performance.

---

## API Contracts

The API is intentionally minimal, but each operation has a clear contract.

- `size()` returns the number of live elements
- `capacity()` returns the amount of owned storage
- `reserve(n)` ensures capacity is at least `n` without changing size
- `clear()` destroys all live elements and keeps storage
- `operator[]` assumes the index is valid
- `push_back` and `emplace_back` may trigger reallocation

A key part of the design is making these rules explicit rather than implicit.

---

## Invalidation Rules

Because storage is contiguous and may be replaced during growth, reallocation invalidates:

- pointers to elements
- references to elements
- iterators, once they exist

Operations that only destroy elements, such as `clear()`, end object lifetime and therefore also invalidate access to those objects.

These rules are part of the container contract, not just an implementation detail.

---

## Performance Model

The container is designed around a simple performance model:

- indexed access is constant time
- `push_back` / `emplace_back` are amortized constant time
- `reserve` is linear in the number of live elements
- `clear` is linear in the number of live elements
- `swap` is constant time

The main performance trade-off is the usual one for dynamic arrays:

- more aggressive growth reduces reallocations
- less aggressive growth reduces memory overhead

This implementation chooses simplicity and predictable behavior over tuning.

---

## Design Trade-offs

### Advantages

- clear ownership model
- explicit lifetime handling
- contiguous layout and good locality
- small, interview-friendly surface area

### Limitations

- reduced feature set compared with `std::vector`
- limited safety beyond documented contracts
- no allocator abstraction
- no attempt at complete standard-library behavior

These limitations are intentional.  
They protect the project from growing beyond its learning goal.

---

## Feature Roadmap

The project is designed to grow in stages, with each stage adding a new concept rather than just more API surface.

### Current stage

- core contiguous storage model
- explicit ownership and lifetime management
- copy and move semantics
- basic insertion and reserve behavior

### Next useful features

- `empty()`, `front()`, `back()`
- `at()` for checked access
- `pop_back()`
- `resize()`

These extend the container without changing its core model.

### Next major design step

- iterator support (`begin()` / `end()`)
- documented iterator invalidation behavior
- const/non-const traversal

This would make the vector more usable while introducing a new layer of API design.

### Performance-oriented extension

- relocation by move during reallocation
- clearer policy around copy vs move costs

This would deepen the performance side of the project.

### Optional advanced extensions

- stronger exception handling
- allocator support
- small buffer optimization

These are intentionally deferred because they add complexity beyond the current learning focus.

---

## End Goal

The end goal is not a feature-complete replacement for `std::vector`.

The end goal is a small vector implementation that is:

- correct for its documented scope
- easy to reason about
- explicit about ownership and lifetime
- strong enough to discuss confidently in a systems interview

---

## Design Philosophy

Prefer a small model that is clear and defensible over a larger model that is harder to reason about.

The value of the project comes from understanding the mechanics, not from maximizing feature count.
