# Design Notes

## Purpose & Context

This project is a learning-focused implementation of arena-style allocators in C++.

The goal is to strengthen fundamentals in:
- memory layout and alignment
- pointer arithmetic and overflow safety
- ownership and object lifetime separation
- API contracts and invariants
- performance-oriented low-level design

The project is not meant to become a general-purpose allocator replacement. Its purpose is to explore a small set of allocator designs deeply and clearly.

---

## Current Design Direction

The current implementation focuses on a simple linear allocator with:

- a fixed-size contiguous buffer
- aligned bump allocation
- constant-time reset
- marker-based rollback for temporary allocations
- raw storage only
- debug-enforced preconditions

The design intentionally favors clarity, explicit contracts, and predictable behavior over feature richness.

---

## Target Feature Set

The broader goal of the project is to explore a small allocator family around arena-based allocation, potentially including:

- aligned linear allocation
- marker / rewind semantics
- scoped temporary allocation helpers
- support for externally provided buffers
- limited typed allocation helpers
- clear documentation of invariants, constraints, and trade-offs

Features should only be added when they deepen understanding of memory, lifetime, ownership, or allocator design.

The project should remain small enough that its core model stays easy to explain and reason about.