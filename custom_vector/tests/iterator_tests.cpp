#include <gtest/gtest.h>
#include "vector.hpp"

TEST(VectorIterator, EmptyVectorBeginEqualsEnd) {
    Vector<int> v;
    EXPECT_EQ(v.begin(), v.end());
}

TEST(VectorIterator, IterationVisitsElementsInOrder) {
    Vector<int> v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    auto it = v.begin();
    ASSERT_NE(it, v.end());
    EXPECT_EQ(*it, 10);

    ++it;
    ASSERT_NE(it, v.end());
    EXPECT_EQ(*it, 20);

    ++it;
    ASSERT_NE(it, v.end());
    EXPECT_EQ(*it, 30);

    ++it;
    EXPECT_EQ(it, v.end());
}

TEST(VectorIterator, ConstBeginEndWork) {
    Vector<int> tmp;
    tmp.push_back(1);
    tmp.push_back(2);
    tmp.push_back(3);

    const Vector<int>& v = tmp;

    auto it = v.begin();
    ASSERT_NE(it, v.end());
    EXPECT_EQ(*it, 1);

    ++it;
    EXPECT_EQ(*it, 2);

    ++it;
    EXPECT_EQ(*it, 3);

    ++it;
    EXPECT_EQ(it, v.end());
}

TEST(VectorIterator, SupportsRangeBasedFor) {
    Vector<int> v;
    v.push_back(4);
    v.push_back(5);
    v.push_back(6);

    int sum = 0;
    for (int x : v) {
        sum += x;
    }

    EXPECT_EQ(sum, 15);
}

TEST(VectorIterator, ReallocationChangesBeginPointer) {
    Vector<int> v;
    v.push_back(1);

    auto old_begin = v.begin();
    const auto old_capacity = v.capacity();

    while (v.capacity() == old_capacity) {
        v.push_back(42);
    }

    EXPECT_NE(old_begin, v.begin());
}