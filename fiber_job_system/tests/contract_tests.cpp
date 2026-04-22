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
    FiberJobSystem scheduler(1);
    EXPECT_THROW(scheduler.submit(std::function<void()>{}), std::invalid_argument);
}

TEST(FiberJobSystemContract, RejectsYieldOutsideRunningFiber)
{
    FiberJobSystem scheduler(1);
    EXPECT_THROW(scheduler.yield_current(), std::logic_error);
}
