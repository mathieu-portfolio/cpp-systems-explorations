#include "arena.hpp"
#include <iostream>
#include <cstdlib>
#include <cstdint>

#define CHECK(cond) \
  do { \
    if (!(cond)) { \
      std::cerr << "CHECK FAILED: " << #cond \
                << " at " << __FILE__ << ":" << __LINE__ \
                << std::endl; \
      std::abort(); \
    } \
  } while (0)

bool is_aligned(void* ptr, size_t alignment)
{
  std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(ptr);
  return (addr % alignment) == 0;
}

void basic_allocation()
{
  Arena arena(32, 8);

  void* p1 = arena.allocate(8, 4);
  CHECK(p1 != nullptr);

  void* p2 = arena.allocate<double>(2);
  CHECK(p2 != nullptr);
}

void alignment()
{
  Arena arena(32, 8);

  void* p = arena.allocate(8, 8);
  CHECK(p != nullptr);
  CHECK(is_aligned(p, 8));
}

void exhaustion()
{
  Arena arena(16, 8);

  CHECK(arena.allocate(8, 4) != nullptr);
  CHECK(arena.allocate(8, 4) != nullptr);
  CHECK(arena.allocate(1, 1) == nullptr);
}

void reset()
{
  Arena arena(16, 8);

  void* a = arena.allocate(8, 4);
  arena.reset();
  void* b = arena.allocate(8, 4);

  CHECK(a == b);
}

void markers()
{
  Arena arena(32, 8);

  auto m = arena.mark();

  void* a = arena.allocate(8, 4);
  arena.rewind(m);
  void* b = arena.allocate(8, 4);

  CHECK(a == b);
}

void nested_markers()
{
  Arena arena(64, 8);

  auto m1 = arena.mark();
  void* a = arena.allocate(8, 4);

  auto m2 = arena.mark();
  void* b = arena.allocate(8, 4);

  arena.rewind(m2);
  void* c = arena.allocate(8, 4);
  CHECK(b == c);

  arena.rewind(m1);
  void* d = arena.allocate(8, 4);
  CHECK(a == d);
}

int main()
{
  basic_allocation();
  alignment();
  exhaustion();
  reset();
  markers();
  nested_markers();

  std::cout << "All tests passed\n";
}