#include "arena.hpp"
#include <cstdint>
#include <new>
#include <cstdio>
#include <cstdlib>

static bool is_power_of_two(size_t x)
{
  return x != 0 && (x & (x - 1)) == 0;
}

[[noreturn]] static void arena_contract_fail(
  const char* file,
  int line,
  const char* expression,
  const char* message = nullptr)
{
  if (message != nullptr)
  {
    std::fprintf(
      stderr,
      "Arena contract violation at %s:%d\n"
      "  expression: %s\n"
      "  message: %s\n",
      file,
      line,
      expression,
      message
    );
  }
  else
  {
    std::fprintf(
      stderr,
      "Arena contract violation at %s:%d\n"
      "  expression: %s\n",
      file,
      line,
      expression
    );
  }

  std::abort();
}

#define ARENA_CHECK_1(expr)                                                     \
  do                                                                            \
  {                                                                             \
    if (!(expr))                                                                \
    {                                                                           \
      arena_contract_fail(__FILE__, __LINE__, #expr);                           \
    }                                                                           \
  } while (false)

#define ARENA_CHECK_2(expr, message)                                            \
  do                                                                            \
  {                                                                             \
    if (!(expr))                                                                \
    {                                                                           \
      arena_contract_fail(__FILE__, __LINE__, #expr, message);                  \
    }                                                                           \
  } while (false)

#define ARENA_CHECK_GET_MACRO(_1, _2, NAME, ...) NAME
#define ARENA_CHECK(...)                                                        \
  ARENA_CHECK_GET_MACRO(__VA_ARGS__, ARENA_CHECK_2, ARENA_CHECK_1)(__VA_ARGS__)

Arena::Arena(size_t capacity, size_t max_alignment)
  : _capacity(capacity), _max_alignment(max_alignment), _offset(0)
{
  ARENA_CHECK(max_alignment != 0, "max_alignment must be non-zero");
  ARENA_CHECK(is_power_of_two(max_alignment), "max_alignment must be a power of two");

  _buffer = static_cast<char*>(
    ::operator new(capacity, std::align_val_t{max_alignment})
  );
}

Arena::~Arena()
{
  ::operator delete(_buffer, std::align_val_t{_max_alignment});
}

void* Arena::allocate(size_t size, size_t alignment)
{
  ARENA_CHECK(alignment != 0, "alignment must be non-zero");
  ARENA_CHECK(is_power_of_two(alignment), "alignment must be a power of two");
  ARENA_CHECK(alignment <= _max_alignment, "alignment exceeds arena max_alignment");

  if (size == 0)
  {
    return nullptr;
  }

  if (_offset >= _capacity)
  {
    return nullptr;
  }

  std::uintptr_t current_address =
    reinterpret_cast<std::uintptr_t>(_buffer + _offset);

  size_t padding = (-current_address) & (alignment - 1);

  if (padding > _capacity - _offset)
  {
    return nullptr;
  }

  size_t aligned_offset = _offset + padding;

  if (size > _capacity - aligned_offset)
  {
    return nullptr;
  }

  size_t new_offset = aligned_offset + size;

  void* ptr = _buffer + aligned_offset;
  _offset = new_offset;
  return ptr;
}

void Arena::reset()
{
  _offset = 0;
}

Arena::Marker Arena::mark() const
{
  return Marker(_offset, this);
}

void Arena::rewind(Arena::Marker marker)
{
  ARENA_CHECK(marker._owner == this, "marker does not belong to this arena");
  ARENA_CHECK(marker._offset <= _offset, "marker offset is not valid for current arena state");

  _offset = marker._offset;
}

Arena::ScopedRewind Arena::scoped_rewind()
{
  return ScopedRewind(this);
}