#include <gtest/gtest.h>
#include "arena.hpp"

TEST(ArenaRewindTest, MarkAndRewindReuseStorage)
{
  Arena arena(32, 8);

  auto marker = arena.mark();

  void* a = arena.allocate(8, 4);
  ASSERT_NE(a, nullptr);
  EXPECT_EQ(arena.used(), 8u);

  arena.rewind(marker);
  EXPECT_EQ(arena.used(), 0u);

  void* b = arena.allocate(8, 4);
  ASSERT_NE(b, nullptr);

  EXPECT_EQ(a, b);
  EXPECT_EQ(arena.used(), 8u);
}

TEST(ArenaRewindTest, NestedMarkersRewindToExpectedPositions)
{
  Arena arena(64, 8);

  auto m1 = arena.mark();
  void* a = arena.allocate(8, 4);
  ASSERT_NE(a, nullptr);

  auto m2 = arena.mark();
  void* b = arena.allocate(8, 4);
  ASSERT_NE(b, nullptr);

  EXPECT_EQ(arena.used(), 16u);

  arena.rewind(m2);
  EXPECT_EQ(arena.used(), 8u);

  void* c = arena.allocate(8, 4);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(b, c);
  EXPECT_EQ(arena.used(), 16u);

  arena.rewind(m1);
  EXPECT_EQ(arena.used(), 0u);

  void* d = arena.allocate(8, 4);
  ASSERT_NE(d, nullptr);
  EXPECT_EQ(a, d);
}

TEST(ArenaRewindTest, RewindingToCurrentMarkerIsNoOp)
{
  Arena arena(32, 8);

  ASSERT_NE(arena.allocate(8, 4), nullptr);
  auto marker = arena.mark();

  const size_t used_before = arena.used();
  arena.rewind(marker);

  EXPECT_EQ(arena.used(), used_before);
  EXPECT_EQ(arena.remaining(), arena.capacity() - used_before);
}

TEST(ArenaRewindTest, ScopedRewindRestoresPreviousStateOnScopeExit)
{
  Arena arena(32, 8);

  ASSERT_NE(arena.allocate(8, 4), nullptr);
  const size_t used_before_scope = arena.used();

  {
    auto scope = arena.scoped_rewind();

    ASSERT_NE(arena.allocate(8, 4), nullptr);
    EXPECT_GT(arena.used(), used_before_scope);
  }

  EXPECT_EQ(arena.used(), used_before_scope);
  EXPECT_EQ(arena.remaining(), arena.capacity() - used_before_scope);
}

TEST(ArenaRewindTest, ScopedRewindDismissKeepsAllocations)
{
  Arena arena(32, 8);

  ASSERT_NE(arena.allocate(8, 4), nullptr);
  const size_t used_before_scope = arena.used();

  size_t used_inside_scope = 0;

  {
    auto scope = arena.scoped_rewind();

    ASSERT_NE(arena.allocate(8, 4), nullptr);
    used_inside_scope = arena.used();

    EXPECT_GT(used_inside_scope, used_before_scope);
    scope.dismiss();
  }

  EXPECT_EQ(arena.used(), used_inside_scope);
  EXPECT_EQ(arena.remaining(), arena.capacity() - used_inside_scope);
}

TEST(ArenaRewindTest, NestedScopedRewindsBehaveLifoOnHappyPath)
{
  Arena arena(64, 8);

  ASSERT_NE(arena.allocate(8, 4), nullptr);
  const size_t used_before_outer = arena.used();

  {
    auto outer = arena.scoped_rewind();
    ASSERT_NE(arena.allocate(8, 4), nullptr);
    const size_t used_before_inner = arena.used();

    {
      auto inner = arena.scoped_rewind();
      ASSERT_NE(arena.allocate(8, 4), nullptr);
      EXPECT_GT(arena.used(), used_before_inner);
    }

    EXPECT_EQ(arena.used(), used_before_inner);
  }

  EXPECT_EQ(arena.used(), used_before_outer);
}

TEST(ArenaRewindTest, ResetAllowsStorageReuse)
{
  Arena arena(32, 8);

  void* a = arena.allocate(8, 4);
  ASSERT_NE(a, nullptr);

  arena.reset();
  EXPECT_EQ(arena.used(), 0u);

  void* b = arena.allocate(8, 4);
  ASSERT_NE(b, nullptr);

  EXPECT_EQ(a, b);
}