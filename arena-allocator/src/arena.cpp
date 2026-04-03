#include "arena.hpp"
#include <cstdint>
#include <new>
#include <cassert>

static bool is_power_of_two(size_t x)
{
  return x != 0 && (x & (x - 1)) == 0;
}

Arena::Arena(size_t capacity, size_t max_alignment)
  : capacity(capacity), max_alignment(max_alignment), offset(0)
{
  // enforce invariant: power-of-two and non-zero
  assert(max_alignment != 0 && is_power_of_two(max_alignment));
  
  buffer = static_cast<char*>(
    ::operator new(capacity, std::align_val_t{max_alignment})
  );
}

Arena::~Arena()
{
  ::operator delete(buffer, std::align_val_t{max_alignment});
}

void* Arena::allocate(size_t size, size_t alignment)
{
  // enforce invariants
  assert(alignment != 0);
  assert(is_power_of_two(alignment));
  assert(alignment <= max_alignment);

  if (size == 0){
    return nullptr;
  }

  std::uintptr_t current_address = reinterpret_cast<std::uintptr_t>(buffer + offset);

  // Compute padding needed to align current address to 'alignment'
  size_t misalignment = current_address % alignment;
  size_t padding = misalignment > 0 ? alignment - misalignment : 0;

  // Ensure adding padding does not overflow or exceed capacity
  if (padding > capacity - offset)
  {
    return nullptr;
  }  
  size_t aligned_offset = offset + padding;

  if (size > capacity - aligned_offset)
  {
    return nullptr;
  }  
  size_t new_offset = aligned_offset + size;
  
  void* ptr = buffer + aligned_offset;
  offset = new_offset;
  return ptr;
}

void Arena::reset()
{
  offset = 0;
}

Arena::Marker Arena::mark() const
{
  return Marker(offset, this);
}

void Arena::rewind(Arena::Marker marker)
{
  // enforce invariants
  assert(marker.offset <= offset);
  assert(marker.owner == this);

  offset = marker.offset;
}