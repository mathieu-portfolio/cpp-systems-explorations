#pragma once

#include <cstdio>
#include <cstdlib>
#include <new>

[[noreturn]] static void vector_contract_fail(
  const char* file,
  int line,
  const char* expression,
  const char* message)
{
  std::fprintf(
    stderr,
    "vector contract violation at %s:%d\n"
    "  expression: %s\n"
    "  message: %s\n",
    file,
    line,
    expression,
    message
  );

  std::abort();
}

#define VECTOR_CHECK(expr, message)                                             \
  do {                                                                          \
    if (!(expr)) {                                                              \
      vector_contract_fail(__FILE__, __LINE__, #expr, message);                 \
    }                                                                           \
  } while (false)


template <typename T>
void Vector<T>::swap(Vector& other) noexcept
{
  std::swap(data_, other.data_);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}

template <typename T>
void Vector<T>::take_from(Vector& other) noexcept
{
  data_ = other.data_;
  size_ = other.size_;
  capacity_ = other.capacity_;

  other.data_ = nullptr;
  other.size_ = 0;
  other.capacity_ = 0;
}

template <typename T>
Vector<T>::Vector() : data_(nullptr), size_(0), capacity_(0) {}

template <typename T>
Vector<T>::Vector(size_type capacity)
  : data_(capacity == 0
      ? nullptr
      : reinterpret_cast<T*>(::operator new(sizeof(T) * capacity))),
    size_(0),
    capacity_(capacity)
{
}

template <typename T>
Vector<T>::~Vector()
{
  clear();
  ::operator delete(data_);
}

template <typename T>
Vector<T>::Vector(const Vector& other)
  : data_(other.capacity_ == 0
      ? nullptr
      : reinterpret_cast<T*>(::operator new(sizeof(T) * other.capacity_))),
    size_(0),
    capacity_(other.capacity_)
{
  for (size_type i = 0 ; i < size_ ; i++)
  {
    new (data_ + i) T(other.data_[i]);
    ++size_;
  }
}

template <typename T>
Vector<T>::Vector(Vector&& other)
  : data_(other.data_),
    size_(other.size_),
    capacity_(other.capacity_)
{
  other.data_ = nullptr;
  other.size_ = 0;
  other.capacity_ = 0;
}

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& other)
{
  if (this == &other) return *this;
  Vector temp(other);
  swap(temp);
  return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector&& other) noexcept
{
  if (this == &other) return *this;
  clear();
  ::operator delete(data_);
  take_from(other);
  return *this;
}

template <typename T>
void Vector<T>::reserve(size_type new_capacity)
{
  if (new_capacity <= capacity_) return;

  T* new_data = new_capacity == 0
    ? nullptr
    : reinterpret_cast<T*>(::operator new(sizeof(T) * new_capacity));
  
  for (size_type i = 0 ; i < size_ ; i++)
  {
    new (new_data + i) T(data_[i]);
  }
  
  for (size_type i = size_ ; i > 0 ; --i)
  {
    data_[i - 1].~T();
  }
  ::operator delete(data_);

  data_ = new_data;
  capacity_ = new_capacity;
}

template <typename T>
void Vector<T>::clear()
{
  for (size_type i = size_ ; i > 0 ; --i)
  {
    data_[i - 1].~T();
  }
  size_ = 0;
}

template <typename T>
void Vector<T>::push_back(const T& value)
{
  emplace_back(value);
}

template <typename T>
void Vector<T>::push_back(T&& value)
{
  emplace_back(std::move(value));
}

template <typename T>
template <typename... Args>
T& Vector<T>::emplace_back(Args&&... args)
{
  if (size_ == capacity_)
  {
    reserve(capacity_ == 0 ? 1 : 2 * capacity_);
  }

  new (data_ + size_) T(std::forward<Args>(args)...);
  size_++;

  return data_[size_ - 1];
}