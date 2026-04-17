#include <gtest/gtest.h>

#include "vector.hpp"

TEST(VectorDeathTest, OperatorIndexFailsOnEmptyVector) {
    Vector<int> v;

    EXPECT_DEATH(
        {
            (void)v[0];
        },
        "operator\\[\\] index out of bounds"
    );
}

TEST(VectorDeathTest, OperatorIndexFailsWhenIndexEqualsSize) {
    Vector<int> v;
    v.push_back(1);
    v.push_back(2);

    EXPECT_DEATH(
        {
            (void)v[2];
        },
        "operator\\[\\] index out of bounds"
    );
}

TEST(VectorDeathTest, ConstOperatorIndexFailsWhenOutOfBounds) {
    Vector<int> tmp;
    tmp.push_back(5);

    const Vector<int>& v = tmp;

    EXPECT_DEATH(
        {
            (void)v[1];
        },
        "operator\\[\\] index out of bounds"
    );
}