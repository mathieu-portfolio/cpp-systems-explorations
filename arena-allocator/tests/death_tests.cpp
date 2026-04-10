#include <gtest/gtest.h>
#include "arena.hpp"

TEST(ArenaDeathTest, ConstructorRejectsZeroMaxAlignment)
{
  EXPECT_DEATH(
    {
      Arena arena(32, 0);
    },
    ""
  );
}

TEST(ArenaDeathTest, ConstructorRejectsNonPowerOfTwoMaxAlignment)
{
  EXPECT_DEATH(
    {
      Arena arena(32, 3);
    },
    ""
  );
}

TEST(ArenaDeathTest, AllocateRejectsZeroAlignment)
{
  EXPECT_DEATH(
    {
      Arena arena(32, 8);
      (void)arena.allocate(8, 0);
    },
    ""
  );
}

TEST(ArenaDeathTest, AllocateRejectsNonPowerOfTwoAlignment)
{
  EXPECT_DEATH(
    {
      Arena arena(32, 8);
      (void)arena.allocate(8, 3);
    },
    ""
  );
}

TEST(ArenaDeathTest, AllocateRejectsAlignmentGreaterThanMaxAlignment)
{
  EXPECT_DEATH(
    {
      Arena arena(32, 8);
      (void)arena.allocate(8, 16);
    },
    ""
  );
}

TEST(ArenaDeathTest, RewindRejectsMarkerFromDifferentArena)
{
  EXPECT_DEATH(
    {
      Arena arena_a(32, 8);
      Arena arena_b(32, 8);
      auto marker_from_a = arena_a.mark();
      arena_b.rewind(marker_from_a);
    },
    ""
  );
}