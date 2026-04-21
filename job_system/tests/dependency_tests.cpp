#include <gtest/gtest.h>

#include "job_system.hpp"

#include <atomic>
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

    jobs.run();
    EXPECT_THROW(jobs.add_dependency(b, a), std::logic_error);
}

TEST(JobSystemDependencies, ExecutesDependentJobAfterPrerequisite)
{
    std::atomic<int> state{0};

    JobSystem jobs(2);

    const JobId a = jobs.create_job([&] {
        state.store(1, std::memory_order_release);
    });

    const JobId b = jobs.create_job([&] {
        EXPECT_EQ(state.load(std::memory_order_acquire), 1);
        state.store(2, std::memory_order_release);
    });

    jobs.add_dependency(b, a);

    jobs.run();
    jobs.wait();

    EXPECT_EQ(state.load(std::memory_order_acquire), 2);
}

TEST(JobSystemDependencies, FanInDependencyRunsOnlyAfterAllPrerequisites)
{
    std::atomic<int> completed_prereqs{0};
    std::atomic<int> observed{0};

    JobSystem jobs(3);

    const JobId a = jobs.create_job([&] {
        completed_prereqs.fetch_add(1, std::memory_order_acq_rel);
    });

    const JobId b = jobs.create_job([&] {
        completed_prereqs.fetch_add(1, std::memory_order_acq_rel);
    });

    const JobId c = jobs.create_job([&] {
        observed.store(completed_prereqs.load(std::memory_order_acquire), std::memory_order_release);
    });

    jobs.add_dependency(c, a);
    jobs.add_dependency(c, b);

    jobs.run();
    jobs.wait();

    EXPECT_EQ(observed.load(std::memory_order_acquire), 2);
}

TEST(JobSystemDependencies, RejectsTwoNodeCycleAtRun)
{
    JobSystem jobs(2);

    const JobId a = jobs.create_job([] {});
    const JobId b = jobs.create_job([] {});

    jobs.add_dependency(a, b);
    jobs.add_dependency(b, a);

    EXPECT_THROW(jobs.run(), std::logic_error);
}

TEST(JobSystemDependencies, RejectsLongerCycleAtRun)
{
    JobSystem jobs(2);

    const JobId a = jobs.create_job([] {});
    const JobId b = jobs.create_job([] {});
    const JobId c = jobs.create_job([] {});

    jobs.add_dependency(a, c);
    jobs.add_dependency(b, a);
    jobs.add_dependency(c, b);

    EXPECT_THROW(jobs.run(), std::logic_error);
}
