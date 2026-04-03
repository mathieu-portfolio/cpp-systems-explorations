#include "arena.hpp"
#include <iostream>
#include <cstdlib>
#include <cstdint>

#define COLOR_RED    "\033[31m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_RESET  "\033[0m"

#define CHECK(cond) \
  do { \
    if (!(cond)) { \
      std::cerr << COLOR_RED \
                << "CHECK FAILED: " << #cond \
                << " at " << __FILE__ << ":" << __LINE__ \
                << COLOR_RESET << std::endl; \
      std::abort(); \
    } \
  } while (0)

void log_pass(const char* test_name)
{
  std::cout << COLOR_GREEN
            << "[PASS] " << test_name
            << COLOR_RESET << '\n';
}

bool is_aligned(void* ptr, size_t alignment)
{
  if (alignment == 0) return false;
  std::uintptr_t current_address = reinterpret_cast<std::uintptr_t>(ptr);
  size_t misalignment = current_address % alignment;
  return misalignment == 0;
}

void basic_allocation_test()
{
  Arena arena(32, 8);

  void* p1 = arena.allocate(8, 4);
  CHECK(p1 != nullptr);

  void* p2 = arena.allocate<double>(2);
  CHECK(p2 != nullptr);

  log_pass(__func__);
}

void alignment_test()
{
  Arena arena(32, 8);

  void* p1 = arena.allocate(8, 4);
  CHECK(p1 != nullptr);
  CHECK(is_aligned(p1, 4));

  void* p2 = arena.allocate(1, 1);
  CHECK(p2 != nullptr);

  void* p3 = arena.allocate(8, 8);
  CHECK(p3 != nullptr);
  CHECK(is_aligned(p3, 8));

  void* p4 = arena.allocate(8, 3);
  CHECK(p4 == nullptr);

  log_pass(__func__);
}

void exhaustion_test()
{
  Arena arena(16, 8);

  void* p1 = arena.allocate(8, 4);
  CHECK(p1 != nullptr);

  void* p2 = arena.allocate(8, 4);
  CHECK(p2 != nullptr);

  void* p3 = arena.allocate(1, 1);
  CHECK(p3 == nullptr);

  log_pass(__func__);
}

void reset_test()
{
  Arena arena(16, 8);

  void* first = arena.allocate(8, 4);
  CHECK(first != nullptr);

  arena.reset();

  void* second = arena.allocate(8, 4);
  CHECK(second != nullptr);

  CHECK(first == second);

  log_pass(__func__);
}

void rewind_reuses_storage_test()
{
  Arena arena(32, 8);

  Arena::Marker marker = arena.mark();

  void* first = arena.allocate(8, 4);
  CHECK(first != nullptr);

  arena.rewind(marker);

  void* second = arena.allocate(8, 4);
  CHECK(second != nullptr);

  CHECK(first == second);

  log_pass(__func__);
}

void nested_markers_test()
{
  Arena arena(64, 8);

  Arena::Marker m1 = arena.mark();

  void* p1 = arena.allocate(8, 4);
  CHECK(p1 != nullptr);

  Arena::Marker m2 = arena.mark();

  void* p2 = arena.allocate(8, 4);
  CHECK(p2 != nullptr);

  arena.rewind(m2);

  void* p3 = arena.allocate(8, 4);
  CHECK(p3 != nullptr);

  CHECK(p2 == p3);

  arena.rewind(m1);

  void* p4 = arena.allocate(8, 4);
  CHECK(p4 != nullptr);

  CHECK(p1 == p4);

  log_pass(__func__);
}

void rewind_preserves_storage_before_marker_test()
{
  Arena arena(64, 8);

  void* before = arena.allocate(8, 4);
  CHECK(before != nullptr);

  Arena::Marker marker = arena.mark();

  void* after = arena.allocate(8, 4);
  CHECK(after != nullptr);

  arena.rewind(marker);

  void* reused = arena.allocate(8, 4);
  CHECK(reused != nullptr);

  CHECK(after == reused);
  CHECK(before != reused);

  log_pass(__func__);
}
int main()
{
  basic_allocation_test();
  alignment_test();
  exhaustion_test();
  reset_test();
  rewind_reuses_storage_test();
  nested_markers_test();
  rewind_preserves_storage_before_marker_test();
}