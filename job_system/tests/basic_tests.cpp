#include <gtest/gtest.h>

#include "job_system.hpp"

#include <atomic>

TEST(JobSystemBasic, CanCreateJobs)
{
    JobSystem jobs(2);

    const JobId a = jobs.create_job([] {});
    const JobId b = jobs.create_job([] {});

    EXPECT_NE(a.value, b.value);
}

TEST(JobSystemBasic, ExecutesIndependentJobs)
{
    std::atomic<int> counter{0};

    JobSystem jobs(2);
    jobs.create_job([&] { counter.fetch_add(1, std::memory_order_relaxed); });
    jobs.create_job([&] { counter.fetch_add(1, std::memory_order_relaxed); });

    jobs.run();
    jobs.wait();

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 2);
}

TEST(JobSystemBasic, EmptyBatchReturnsImmediately)
{
    JobSystem jobs(2);

    jobs.run();
    jobs.wait();

    SUCCEED();
}
