#include "arena.hpp"
#include <cstdint>

Arena::Arena(size_t capacity)
  : capacity(capacity), offset(0)
{
  buffer = new char[capacity];
}

Arena::~Arena()
{
  delete[] buffer;
}

static bool is_power_of_two(size_t x)
{
  return x != 0 && (x & (x - 1)) == 0;
}

void* Arena::allocate(size_t size, size_t alignment)
{
  if (size == 0)
  {
    return nullptr;
  }

  // Only power-of-two alignments are supported
  if (!is_power_of_two(alignment))
  {
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