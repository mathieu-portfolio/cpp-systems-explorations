#pragma once

#include <cstddef>
#include <utility>

template <typename T>
class Vector {
private:
    // Invariants:
    // - 0 <= size_ <= capacity_
    // - if capacity_ == 0, data_ == nullptr
    // - elements in [0, size_) are constructed
    // - slots in [size_, capacity_) are allocated storage only
    T* data_;
    std::size_t size_;
    std::size_t capacity_;

    void assert_invariants() const;
    void swap(Vector& other) noexcept;
    void take_from(Vector& other) noexcept;

public:
    using value_type = T;
    using size_type = std::size_t;

    Vector();
    explicit Vector(size_type capacity);
    ~Vector();

    Vector(const Vector& other);
    Vector(Vector&& other) noexcept;

    Vector& operator=(const Vector& other);
    Vector& operator=(Vector&& other) noexcept;

    size_type size() const { return size_; }
    size_type capacity() const { return capacity_; }
    void reserve(size_type new_capacity);
    void clear();

    T& operator[](size_type index);
    const T& operator[](size_type index) const;

    void push_back(const T& value);
    void push_back(T&& value);

    template <typename... Args>
    T& emplace_back(Args&&... args);
};

#include "vector.inl"