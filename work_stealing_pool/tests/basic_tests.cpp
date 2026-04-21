#include <gtest/gtest.h>

#include "work_stealing_pool.hpp"

TEST(WorkStealingPoolBasic, CanConstructPool)
{
    WorkStealingPool pool(2);
    SUCCEED();
}
