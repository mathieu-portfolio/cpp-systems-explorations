#include <gtest/gtest.h>

#include "vector.hpp"

#include "test_utils.hpp"

TEST(VectorLifetime, ClearDestroysLiveElementsAndKeepsCapacity) {
    Tracked::reset();

    Vector<Tracked> v;
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);

    ASSERT_EQ(Tracked::alive, 3);

    const auto cap_before = v.capacity();

    v.clear();

    EXPECT_EQ(v.size(), 0u);
    EXPECT_EQ(v.capacity(), cap_before);
    EXPECT_EQ(Tracked::alive, 0);
    EXPECT_GE(Tracked::dtor, 3);
}

TEST(VectorLifetime, DestructorDestroysRemainingLiveElementsOnce) {
    Tracked::reset();

    {
        Vector<Tracked> v;
        v.emplace_back(10);
        v.emplace_back(20);

        ASSERT_EQ(Tracked::alive, 2);
    }

    EXPECT_EQ(Tracked::alive, 0);
    EXPECT_GE(Tracked::dtor, 2);
}

TEST(VectorLifetime, ReserveRelocatesExistingElementsAndDestroysOldOnes) {
    Tracked::reset();

    Vector<Tracked> v;
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);

    ASSERT_EQ(Tracked::alive, 3);

    v.reserve(32);

    EXPECT_EQ(v.size(), 3u);
    EXPECT_GE(v.capacity(), 32u);
    EXPECT_EQ(v[0].value, 1);
    EXPECT_EQ(v[1].value, 2);
    EXPECT_EQ(v[2].value, 3);

    EXPECT_EQ(Tracked::alive, 3);
    EXPECT_GE(Tracked::copy_ctor + Tracked::move_ctor, 3);
}

TEST(VectorLifetime, MoveConstructionDoesNotDoubleDestroyTransferredElements) {
    Tracked::reset();

    {
        Vector<Tracked> src;
        src.emplace_back(1);
        src.emplace_back(2);

        {
            Vector<Tracked> dst(std::move(src));
            ASSERT_EQ(Tracked::alive, 2);
            EXPECT_EQ(dst.size(), 2u);
            EXPECT_EQ(src.size(), 0u);
            EXPECT_EQ(src.capacity(), 0u);
        }

        EXPECT_EQ(Tracked::alive, 0);
    }

    EXPECT_EQ(Tracked::alive, 0);
}