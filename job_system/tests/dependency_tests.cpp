#include <gtest/gtest.h>

#include "job_system.hpp"

TEST(JobSystemDependencies, AddDependencyIsNotImplementedYet)
{
    JobSystem jobs(2);

    const JobId a = jobs.create_job([] {});
    const JobId b = jobs.create_job([] {});

    EXPECT_THROW(jobs.add_dependency(b, a), std::logic_error);
}
