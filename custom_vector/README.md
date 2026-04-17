# Custom Vector

**Version:** v1.0

## Problem

Dynamic arrays are a fundamental data structure in systems programming.

Naive implementations using raw pointers or manual allocation often lead to:

- unsafe memory management
- unclear ownership
- undefined behavior
- inefficient reallocations

While `std::vector` solves these problems, it hides the underlying mechanics.

This project aims to make those mechanics explicit.

---

## Solution

This project implements a minimal `std::vector`-like container with:

- contiguous storage
- manual memory management
- explicit object lifetime control
- predictable growth strategy

> Store objects in contiguous memory and manage their lifetime explicitly.

---

## Key Idea

The vector separates:

- raw memory allocation
- object construction and destruction

This allows precise control over:

- when objects exist
- when memory is reused
- how ownership is transferred

---

## Usage

### Emplace

```cpp
v.emplace_back(args...);
```

Constructs an element directly in-place.

---

### Push Back

```cpp
v.push_back(value);
v.push_back(std::move(value));
```

Adds an element using copy or move semantics.

---

### Reserve

```cpp
v.reserve(n);
```

Ensures capacity without changing size.

---

### Clear

```cpp
v.clear();
```

Destroys all elements but keeps allocated storage.

---

## Scope

This project focuses on:

- correctness
- memory model understanding
- ownership and lifetime
- performance fundamentals

---

## Limitations

- not STL-compatible
- no allocator support
- minimal exception handling
- no iterator support
- single-threaded

---

## Project Structure

- `vector` — implementation
- `demo` — usage examples
- `tests` — unit tests for invariants, lifetime, copy/move behavior, and reallocation

---

## Notes

This is a learning-focused implementation.

For detailed design and technical reasoning, see `DESIGN.md`.
