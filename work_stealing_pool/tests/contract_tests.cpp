#include <gtest/gtest.h>

#include "work_stealing_pool.hpp"

#include <functional>
#include <stdexcept>

TEST(WorkStealingPoolContract, RejectsZeroWorkers)
{
    EXPECT_THROW(WorkStealingPool pool(0), std::invalid_argument);
}

TEST(WorkStealingPoolContract, RejectsEmptyJobSubmission)
{
    WorkStealingPool pool(2);
    EXPECT_THROW(pool.submit(std::function<void()>{}), std::invalid_argument);
}
