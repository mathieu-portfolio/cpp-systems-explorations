#include <gtest/gtest.h>

#include "job_system.hpp"

TEST(JobSystemWait, WaitIsNotImplementedYet)
{
    JobSystem jobs(2);

    EXPECT_THROW(jobs.wait(), std::logic_error);
}
