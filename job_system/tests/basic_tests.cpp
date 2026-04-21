#include <gtest/gtest.h>

#include "job_system.hpp"

TEST(JobSystemBasic, CanCreateJobs)
{
    JobSystem jobs(2);

    const JobId a = jobs.create_job([] {});
    const JobId b = jobs.create_job([] {});

    EXPECT_NE(a.value, b.value);
}
