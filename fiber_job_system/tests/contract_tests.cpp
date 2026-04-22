#include <gtest/gtest.h>

#include "fiber_job_system.hpp"

#include <functional>
#include <stdexcept>

TEST(FiberJobSystemContract, RejectsZeroWorkers)
{
    EXPECT_THROW(FiberJobSystem scheduler(0), std::invalid_argument);
}

TEST(FiberJobSystemContract, RejectsEmptySubmission)
{
    FiberJobSystem scheduler(2);
    EXPECT_THROW(scheduler.submit(std::function<void()>{}), std::invalid_argument);
}
