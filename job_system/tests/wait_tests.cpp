#include <gtest/gtest.h>

#include "job_system.hpp"

#include <atomic>
#include <chrono>
#include <thread>

TEST(JobSystemWait, WaitReturnsAfterSubmittedWorkFinishes)
{
    std::atomic<int> counter{0};

    JobSystem jobs(2);

    jobs.create_job([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        counter.fetch_add(1, std::memory_order_relaxed);
    });

    jobs.create_job([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        counter.fetch_add(1, std::memory_order_relaxed);
    });

    jobs.run();
    jobs.wait();

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 2);
}
