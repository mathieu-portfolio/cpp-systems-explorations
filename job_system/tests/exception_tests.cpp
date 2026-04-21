#include <gtest/gtest.h>

#include "job_system.hpp"

#include <atomic>
#include <stdexcept>

TEST(JobSystemExceptions, ThrowingJobDoesNotBlockBatchCompletion)
{
    std::atomic<int> counter{0};

    JobSystem jobs(2);

    jobs.create_job([] {
        throw std::runtime_error("job failure");
    });

    jobs.create_job([&] {
        counter.fetch_add(1, std::memory_order_relaxed);
    });

    jobs.run();
    jobs.wait();

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 1);
}

TEST(JobSystemExceptions, DependentsStillRunAfterThrowingPrerequisite)
{
    std::atomic<int> state{0};

    JobSystem jobs(2);

    const JobId a = jobs.create_job([&] {
        state.store(1, std::memory_order_release);
        throw std::runtime_error("job failure");
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
