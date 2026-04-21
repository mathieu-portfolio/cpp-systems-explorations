#include <gtest/gtest.h>

#include "job_system.hpp"

#include <functional>
#include <stdexcept>

TEST(JobSystemContract, RejectsEmptyJob)
{
    JobSystem jobs(2);

    EXPECT_THROW(
        {
            jobs.create_job(std::function<void()>{});
        },
        std::invalid_argument);
}

TEST(JobSystemContract, RejectsCreatingJobsAfterRunStarts)
{
    JobSystem jobs(2);

    jobs.run();

    EXPECT_THROW(
        {
            jobs.create_job([] {});
        },
        std::logic_error);
}

TEST(JobSystemContract, RejectsRunMoreThanOnce)
{
    JobSystem jobs(2);

    jobs.run();
    EXPECT_THROW(jobs.run(), std::logic_error);
}

TEST(JobSystemContract, FailedRunOnCycleDoesNotFreezeGraphMutation)
{
    JobSystem jobs(2);

    const JobId a = jobs.create_job([] {});
    const JobId b = jobs.create_job([] {});

    jobs.add_dependency(a, b);
    jobs.add_dependency(b, a);

    EXPECT_THROW(jobs.run(), std::logic_error);
    EXPECT_NO_THROW(jobs.create_job([] {}));
}
