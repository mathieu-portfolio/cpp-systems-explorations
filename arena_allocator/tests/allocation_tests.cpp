#include <gtest/gtest.h>
#include <cstdint>
#include "arena.hpp"

namespace
{
bool is_aligned(void* ptr, size_t alignment)
{
  auto addr = reinterpret_cast<std::uintptr_t>(ptr);
  return (addr % alignment) == 0;
}
}

TEST(ArenaAllocationTest, BasicRawAllocationSucceeds)
{
  Arena arena(32, 8);

  void* p = arena.allocate(8, 4);

  ASSERT_NE(p, nullptr);
  EXPECT_TRUE(is_aligned(p, 4));
}

TEST(ArenaAllocationTest, TypedAllocationSucceedsAndIsAligned)
{
  Arena arena(64, 8);

  void* p = arena.allocate<double>(2);

  ASSERT_NE(p, nullptr);
  EXPECT_TRUE(is_aligned(p, alignof(double)));
}

TEST(ArenaAllocationTest, ZeroSizeRawAllocationReturnsNullptr)
{
  Arena arena(32, 8);

  EXPECT_EQ(arena.allocate(0, 1), nullptr);
  EXPECT_EQ(arena.used(), 0u);
  EXPECT_EQ(arena.remaining(), 32u);
}

TEST(ArenaAllocationTest, ZeroCountTypedAllocationReturnsNullptr)
{
  Arena arena(32, 8);

  EXPECT_EQ(arena.allocate<int>(0), nullptr);
  EXPECT_EQ(arena.used(), 0u);
  EXPECT_EQ(arena.remaining(), 32u);
}

TEST(ArenaAllocationTest, ExactFitSucceeds)
{
  Arena arena(16, 8);

  void* p1 = arena.allocate(8, 4);
  void* p2 = arena.allocate(8, 4);

  ASSERT_NE(p1, nullptr);
  ASSERT_NE(p2, nullptr);
  EXPECT_EQ(arena.used(), 16u);
  EXPECT_EQ(arena.remaining(), 0u);
}

TEST(ArenaAllocationTest, ExhaustionReturnsNullptrAndDoesNotChangeState)
{
  Arena arena(16, 8);

  ASSERT_NE(arena.allocate(8, 4), nullptr);
  ASSERT_NE(arena.allocate(8, 4), nullptr);

  const size_t used_before = arena.used();
  const size_t remaining_before = arena.remaining();

  EXPECT_EQ(arena.allocate(1, 1), nullptr);
  EXPECT_EQ(arena.used(), used_before);
  EXPECT_EQ(arena.remaining(), remaining_before);
}

TEST(ArenaAllocationTest, AlignmentPaddingIsAppliedCorrectly)
{
  Arena arena(32, 8);

  void* p1 = arena.allocate(1, 1);
  ASSERT_NE(p1, nullptr);
  EXPECT_EQ(arena.used(), 1u);

  void* p2 = arena.allocate(8, 8);
  ASSERT_NE(p2, nullptr);
  EXPECT_TRUE(is_aligned(p2, 8));

  // Base buffer is aligned to max_alignment == 8, so after 1 byte,
  // 7 bytes of padding are required before the 8-byte aligned block.
  EXPECT_EQ(arena.used(), 16u);
  EXPECT_EQ(arena.remaining(), 16u);
}

TEST(ArenaAllocationTest, FailureAfterPaddingRequirementDoesNotChangeState)
{
  Arena arena(8, 8);

  ASSERT_NE(arena.allocate(1, 1), nullptr);
  EXPECT_EQ(arena.used(), 1u);

  const size_t used_before = arena.used();
  const size_t remaining_before = arena.remaining();

  // Would require 7 bytes padding + 8 bytes payload = 15 total, which does not fit.
  EXPECT_EQ(arena.allocate(8, 8), nullptr);
  EXPECT_EQ(arena.used(), used_before);
  EXPECT_EQ(arena.remaining(), remaining_before);
}

TEST(ArenaAllocationTest, TypedAllocationOverflowReturnsNullptrAndDoesNotChangeState)
{
  Arena arena(64, 8);

  const size_t used_before = arena.used();
  const size_t remaining_before = arena.remaining();

  constexpr size_t huge_count =
    static_cast<size_t>(-1) / sizeof(std::uint64_t) + 1;

  EXPECT_EQ(arena.allocate<std::uint64_t>(huge_count), nullptr);
  EXPECT_EQ(arena.used(), used_before);
  EXPECT_EQ(arena.remaining(), remaining_before);
}