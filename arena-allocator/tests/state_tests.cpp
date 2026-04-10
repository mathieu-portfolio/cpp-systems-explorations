#include <gtest/gtest.h>
#include "arena.hpp"

TEST(ArenaStateTest, InitialStateIsConsistent)
{
  Arena arena(32, 8);

  EXPECT_EQ(arena.capacity(), 32u);
  EXPECT_EQ(arena.max_alignment(), 8u);
  EXPECT_EQ(arena.used(), 0u);
  EXPECT_EQ(arena.remaining(), 32u);
}

TEST(ArenaStateTest, StateUpdatesAfterSimpleAllocations)
{
  Arena arena(32, 8);

  ASSERT_NE(arena.allocate(8, 4), nullptr);
  EXPECT_EQ(arena.used(), 8u);
  EXPECT_EQ(arena.remaining(), 24u);

  ASSERT_NE(arena.allocate(4, 4), nullptr);
  EXPECT_EQ(arena.used(), 12u);
  EXPECT_EQ(arena.remaining(), 20u);
}

TEST(ArenaStateTest, StateTracksAlignmentPadding)
{
  Arena arena(32, 8);

  ASSERT_NE(arena.allocate(1, 1), nullptr);
  EXPECT_EQ(arena.used(), 1u);
  EXPECT_EQ(arena.remaining(), 31u);

  ASSERT_NE(arena.allocate(8, 8), nullptr);
  EXPECT_EQ(arena.used(), 16u);
  EXPECT_EQ(arena.remaining(), 16u);
}

TEST(ArenaStateTest, FailedAllocationLeavesStateUnchanged)
{
  Arena arena(16, 8);

  ASSERT_NE(arena.allocate(16, 1), nullptr);
  EXPECT_EQ(arena.used(), 16u);
  EXPECT_EQ(arena.remaining(), 0u);

  EXPECT_EQ(arena.allocate(1, 1), nullptr);
  EXPECT_EQ(arena.used(), 16u);
  EXPECT_EQ(arena.remaining(), 0u);
}

TEST(ArenaStateTest, ResetRestoresInitialState)
{
  Arena arena(32, 8);

  ASSERT_NE(arena.allocate(8, 4), nullptr);
  ASSERT_NE(arena.allocate(8, 8), nullptr);

  ASSERT_GT(arena.used(), 0u);
  ASSERT_LT(arena.remaining(), arena.capacity());

  arena.reset();

  EXPECT_EQ(arena.used(), 0u);
  EXPECT_EQ(arena.remaining(), arena.capacity());
}

TEST(ArenaStateTest, UsedPlusRemainingAlwaysEqualsCapacityOnObservedStates)
{
  Arena arena(32, 8);

  EXPECT_EQ(arena.used() + arena.remaining(), arena.capacity());

  ASSERT_NE(arena.allocate(1, 1), nullptr);
  EXPECT_EQ(arena.used() + arena.remaining(), arena.capacity());

  ASSERT_NE(arena.allocate(8, 8), nullptr);
  EXPECT_EQ(arena.used() + arena.remaining(), arena.capacity());

  arena.reset();
  EXPECT_EQ(arena.used() + arena.remaining(), arena.capacity());
}