#include <gtest/gtest.h>

#include "fiber_job_system.hpp"

TEST(FiberJobSystemBasic, CanConstructScheduler)
{
    FiberJobSystem scheduler(2);
    SUCCEED();
}
