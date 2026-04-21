# Design Notes

## Scope

### In scope

- contiguous storage
- push_back / emplace_back
- reserve / size / capacity
- operator[]
- clear / destructor
- copy and move semantics
- basic iterator support (begin/end)

## Invariants

- 0 <= size_ <= capacity_
- data_ == nullptr iff capacity_ == 0
- [0, size_) constructed
- [size_, capacity_) raw storage

## Reallocation

1. allocate new buffer
2. move-construct elements into new buffer
3. destroy old elements
4. free old buffer

Relocation uses move semantics.

## Iteration Model

Iterators are raw pointers:

- begin() returns data_
- end() returns data_ + size_

Supports:

- manual iteration
- range-based for

## Invalidation Rules

The following operations invalidate all iterators, pointers, and references:

- reserve (if capacity grows)
- push_back / emplace_back (if reallocation occurs)
- clear()

Operations that do not reallocate preserve iterator validity.

## Design Philosophy

Small, explicit, and easy to reason about.
