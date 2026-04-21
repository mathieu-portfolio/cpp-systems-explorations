# Custom Vector

**Version:** v2.0

## Problem

Dynamic arrays are a fundamental data structure in systems programming.

Naive implementations using raw pointers or manual allocation often lead to:

- unsafe memory management
- unclear ownership
- undefined behavior
- inefficient reallocations

While `std::vector` solves these problems, it hides the underlying mechanics.

This project aims to make those mechanics explicit.

## Solution

This project implements a minimal `std::vector`-like container with:

- contiguous storage
- manual memory management
- explicit object lifetime control
- predictable growth strategy
- basic iterator support

## Usage

### Insertion

```cpp
Vector<int> v;
v.emplace_back(1);
v.emplace_back(2);
```

### Iteration

```cpp
for (const auto& x : v) {
    // use x
}
```

### Access

```cpp
v[0] = 42;
```

### Reserve

```cpp
v.reserve(10);
```

## Scope

- correctness
- memory model understanding
- ownership and lifetime
- performance fundamentals

## Limitations

- not STL-compatible
- no allocator support
- minimal exception handling
- no advanced iterator model
- single-threaded

## Project Structure

- `vector` — implementation
- `demo` — usage examples
- `tests` — unit tests

## Notes

This is a learning-focused implementation.
