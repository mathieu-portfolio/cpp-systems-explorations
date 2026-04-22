#include <gtest/gtest.h>

#include "fiber_job_system.hpp"

#include <atomic>
#include <chrono>
#include <thread>

TEST(FiberJobSystemScheduling, ExecutesSubmittedWork)
{
    std::atomic<int> counter{0};

    {
        FiberJobSystem scheduler(2);
        scheduler.submit([&] {
            counter.fetch_add(1, std::memory_order_relaxed);
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 1);
}

TEST(FiberJobSystemScheduling, YieldSuspendsAndResumeRequeuesFiber)
{
    std::atomic<int> runs{0};

    {
        FiberJobSystem scheduler(2);

        scheduler.submit([&] {
            const int current = runs.fetch_add(1, std::memory_order_relaxed);
            if (current == 0)
            {
                scheduler.yield_current();
            }
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        EXPECT_EQ(runs.load(std::memory_order_relaxed), 1);

        scheduler.resume_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    EXPECT_EQ(runs.load(std::memory_order_relaxed), 2);
}

TEST(FiberJobSystemScheduling, OtherWorkCanRunWhileFiberIsSuspended)
{
    std::atomic<int> counter{0};

    {
        FiberJobSystem scheduler(2);

        scheduler.submit([&] {
            scheduler.yield_current();
        });

        scheduler.submit([&] {
            counter.fetch_add(1, std::memory_order_relaxed);
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 1);
}
