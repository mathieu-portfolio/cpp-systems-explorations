#include <gtest/gtest.h>

#include "job_system.hpp"

#include <stdexcept>

TEST(JobSystemDependencies, CanAddSingleDependencyBeforeRun)
{
    JobSystem jobs(2);

    const JobId a = jobs.create_job([] {});
    const JobId b = jobs.create_job([] {});

    EXPECT_NO_THROW(jobs.add_dependency(b, a));
}

TEST(JobSystemDependencies, CanAddMultipleDependenciesToSameJob)
{
    JobSystem jobs(2);

    const JobId a = jobs.create_job([] {});
    const JobId b = jobs.create_job([] {});
    const JobId c = jobs.create_job([] {});

    EXPECT_NO_THROW(jobs.add_dependency(c, a));
    EXPECT_NO_THROW(jobs.add_dependency(c, b));
}

TEST(JobSystemDependencies, RejectsOutOfRangeDependentJobId)
{
    JobSystem jobs(2);

    const JobId a = jobs.create_job([] {});

    EXPECT_THROW(jobs.add_dependency(JobId{999}, a), std::out_of_range);
}

TEST(JobSystemDependencies, RejectsOutOfRangePrerequisiteJobId)
{
    JobSystem jobs(2);

    const JobId a = jobs.create_job([] {});

    EXPECT_THROW(jobs.add_dependency(a, JobId{999}), std::out_of_range);
}

TEST(JobSystemDependencies, RejectsSelfDependency)
{
    JobSystem jobs(2);

    const JobId a = jobs.create_job([] {});

    EXPECT_THROW(jobs.add_dependency(a, a), std::invalid_argument);
}

TEST(JobSystemDependencies, RejectsAddingDependencyAfterRunStarts)
{
    JobSystem jobs(2);

    const JobId a = jobs.create_job([] {});
    const JobId b = jobs.create_job([] {});

    EXPECT_THROW(jobs.run(), std::logic_error);
    EXPECT_THROW(jobs.add_dependency(b, a), std::logic_error);
}
