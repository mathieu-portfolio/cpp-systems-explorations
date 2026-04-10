#include <gtest/gtest.h>
#include "arena.hpp"

TEST(ArenaDeathTest, ConstructorRejectsZeroMaxAlignment)
{
  EXPECT_DEATH(
    {
      Arena arena(32, 0);
    },
    "max_alignment must be non-zero"
  );
}

TEST(ArenaDeathTest, ConstructorRejectsNonPowerOfTwoMaxAlignment)
{
  EXPECT_DEATH(
    {
      Arena arena(32, 3);
    },
    "max_alignment must be a power of two"
  );
}

TEST(ArenaDeathTest, AllocateRejectsZeroAlignment)
{
  Arena arena(32, 8);

  EXPECT_DEATH(
    {
      (void)arena.allocate(8, 0);
    },
    "alignment must be non-zero"
  );
}

TEST(ArenaDeathTest, AllocateRejectsNonPowerOfTwoAlignment)
{
  Arena arena(32, 8);

  EXPECT_DEATH(
    {
      (void)arena.allocate(8, 3);
    },
    "alignment must be a power of two"
  );
}

TEST(ArenaDeathTest, AllocateRejectsAlignmentGreaterThanMaxAlignment)
{
  Arena arena(32, 8);

  EXPECT_DEATH(
    {
      (void)arena.allocate(8, 16);
    },
    "alignment exceeds arena max_alignment"
  );
}

TEST(ArenaDeathTest, RewindRejectsMarkerFromDifferentArena)
{
  Arena arena_a(32, 8);
  Arena arena_b(32, 8);

  auto marker_from_a = arena_a.mark();

  EXPECT_DEATH(
    {
      arena_b.rewind(marker_from_a);
    },
    "marker does not belong to this arena"
  );
}