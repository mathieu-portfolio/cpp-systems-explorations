#include <gtest/gtest.h>

#include "vector.hpp"

#include "test_utils.hpp"

TEST(VectorBasic, DefaultConstructedIsEmpty) {
    Vector<int> v;

    EXPECT_EQ(v.size(), 0u);
    EXPECT_EQ(v.capacity(), 0u);
}

TEST(VectorBasic, PushBackSingleElement) {
    Vector<int> v;

    v.push_back(42);

    ASSERT_EQ(v.size(), 1u);
    EXPECT_GE(v.capacity(), 1u);
    EXPECT_EQ(v[0], 42);
}

TEST(VectorBasic, PushBackMultiplePreservesOrder) {
    Vector<int> v;

    v.push_back(10);
    v.push_back(20);
    v.push_back(30);
    v.push_back(40);

    ASSERT_EQ(v.size(), 4u);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 30);
    EXPECT_EQ(v[3], 40);
}

TEST(VectorBasic, EmplaceBackConstructsInPlace) {
    Vector<PairLike> v;

    PairLike& ref = v.emplace_back(3, 7);

    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0].a, 3);
    EXPECT_EQ(v[0].b, 7);
    EXPECT_EQ(&ref, &v[0]);
}

TEST(VectorBasic, ReserveIncreasesCapacityWithoutChangingSize) {
    Vector<int> v;
    v.push_back(1);
    v.push_back(2);

    const auto old_size = v.size();

    v.reserve(16);

    EXPECT_EQ(v.size(), old_size);
    EXPECT_GE(v.capacity(), 16u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
}

TEST(VectorBasic, ReserveSmallerThanCapacityDoesNothingToContents) {
    Vector<int> v;
    v.push_back(5);
    v.push_back(6);
    v.push_back(7);

    const auto old_capacity = v.capacity();

    v.reserve(1);

    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v.capacity(), old_capacity);
    EXPECT_EQ(v[0], 5);
    EXPECT_EQ(v[1], 6);
    EXPECT_EQ(v[2], 7);
}

TEST(VectorBasic, ReallocationChangesStorageAddress) {
    Vector<int> v;
    v.push_back(1);

    int* old_ptr = &v[0];
    const auto old_capacity = v.capacity();

    while (v.capacity() == old_capacity) {
        v.push_back(123);
    }

    int* new_ptr = &v[0];

    EXPECT_NE(old_ptr, new_ptr);
}

TEST(VectorBasic, OperatorIndexReturnsMutableReference) {
    Vector<int> v;
    v.push_back(10);
    v.push_back(20);

    v[1] = 99;

    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 99);
}

TEST(VectorBasic, OperatorIndexConstOverloadReturnsConstReference) {
    Vector<int> tmp;
    tmp.push_back(7);
    tmp.push_back(8);

    const Vector<int>& v = tmp;

    EXPECT_EQ(v[0], 7);
    EXPECT_EQ(v[1], 8);
}