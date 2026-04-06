#include "arena.hpp"
#include <cstdint>
#include <new>
#include <cassert>

static bool is_power_of_two(size_t x)
{
  return x != 0 && (x & (x - 1)) == 0;
}

Arena::Arena(size_t capacity, size_t max_alignment)
  : _capacity(capacity), _max_alignment(max_alignment), _offset(0)
{
  // enforce invariant: power-of-two and non-zero
  assert(max_alignment != 0 && is_power_of_two(max_alignment));
  
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
  // enforce invariants
  assert(alignment != 0);
  assert(is_power_of_two(alignment));
  assert(alignment <= _max_alignment);

  if (size == 0)
  {
    return nullptr;
  }

  if (_offset >= _capacity)
  {
    return nullptr;
  }

  std::uintptr_t current_address = reinterpret_cast<std::uintptr_t>(_buffer + _offset);

  // Compute padding needed to align current address to 'alignment'
  size_t misalignment = current_address % alignment;
  size_t padding = misalignment > 0 ? alignment - misalignment : 0;

  // Ensure adding padding does not overflow or exceed capacity
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
  // enforce invariants
  assert(marker._offset <= _offset);
  assert(marker._owner == this);

  _offset = marker._offset;
}
