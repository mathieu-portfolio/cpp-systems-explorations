#include <gtest/gtest.h>

#include "fiber_job_system.hpp"

TEST(FiberJobSystemScheduling, SubmitIsNotImplementedYet)
{
    FiberJobSystem scheduler(2);
    EXPECT_THROW(scheduler.submit([] {}), std::logic_error);
}

TEST(FiberJobSystemScheduling, YieldCurrentIsNotImplementedYet)
{
    FiberJobSystem scheduler(2);
    EXPECT_THROW(scheduler.yield_current(), std::logic_error);
}
