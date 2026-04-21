#include <gtest/gtest.h>

#include "work_stealing_pool.hpp"

TEST(WorkStealingPoolSubmission, SubmitIsNotImplementedYet)
{
    WorkStealingPool pool(2);
    EXPECT_THROW(pool.submit([] {}), std::logic_error);
}
