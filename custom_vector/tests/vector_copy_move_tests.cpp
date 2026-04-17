#include <gtest/gtest.h>
#include "vector.hpp"

TEST(VectorCopyMove, CopyConstructorCopiesSizeAndValues) {
    Vector<int> src;
    src.push_back(1);
    src.push_back(2);
    src.push_back(3);

    Vector<int> copy(src);

    ASSERT_EQ(copy.size(), 3u);
    EXPECT_EQ(copy[0], 1);
    EXPECT_EQ(copy[1], 2);
    EXPECT_EQ(copy[2], 3);
}

TEST(VectorCopyMove, CopyProducesIndependentStorage) {
    Vector<int> src;
    src.push_back(11);
    src.push_back(22);

    Vector<int> copy(src);
    copy[0] = 99;

    EXPECT_EQ(src.size(), 2u);
    EXPECT_EQ(copy.size(), 2u);

    EXPECT_EQ(src[0], 11);
    EXPECT_EQ(copy[0], 99);
}

TEST(VectorCopyMove, CopyAssignmentCopiesValues) {
    Vector<int> src;
    src.push_back(4);
    src.push_back(5);
    src.push_back(6);

    Vector<int> dst;
    dst.push_back(100);

    dst = src;

    ASSERT_EQ(dst.size(), 3u);
    EXPECT_EQ(dst[0], 4);
    EXPECT_EQ(dst[1], 5);
    EXPECT_EQ(dst[2], 6);
}

TEST(VectorCopyMove, CopyAssignmentSelfAssignmentIsSafe) {
    Vector<int> v;
    v.push_back(9);
    v.push_back(8);

    v = v;

    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 9);
    EXPECT_EQ(v[1], 8);
}

TEST(VectorCopyMove, MoveConstructorTransfersOwnership) {
    Vector<int> src;
    src.push_back(7);
    src.push_back(8);
    src.push_back(9);

    Vector<int> moved(std::move(src));

    ASSERT_EQ(moved.size(), 3u);
    EXPECT_EQ(moved[0], 7);
    EXPECT_EQ(moved[1], 8);
    EXPECT_EQ(moved[2], 9);

    EXPECT_EQ(src.size(), 0u);
    EXPECT_EQ(src.capacity(), 0u);
}

TEST(VectorCopyMove, MoveAssignmentTransfersOwnership) {
    Vector<int> src;
    src.push_back(1);
    src.push_back(2);

    Vector<int> dst;
    dst.push_back(99);
    dst.push_back(100);

    dst = std::move(src);

    ASSERT_EQ(dst.size(), 2u);
    EXPECT_EQ(dst[0], 1);
    EXPECT_EQ(dst[1], 2);

    EXPECT_EQ(src.size(), 0u);
    EXPECT_EQ(src.capacity(), 0u);
}

TEST(VectorCopyMove, MoveAssignmentSelfAssignmentIsSafe) {
    Vector<int> v;
    v.push_back(12);
    v.push_back(13);

    v = std::move(v);

    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 12);
    EXPECT_EQ(v[1], 13);
}