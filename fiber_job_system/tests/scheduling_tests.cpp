#include <gtest/gtest.h>

#include "fiber_job_system.hpp"

#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

TEST(FiberJobSystemScheduling, ExecutesSubmittedWork)
{
    int counter = 0;
    std::mutex mutex;

    {
        FiberJobSystem scheduler(1);
        scheduler.submit([&] {
            std::lock_guard<std::mutex> lock(mutex);
            ++counter;
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    EXPECT_EQ(counter, 1);
}

TEST(FiberJobSystemScheduling, FiberResumesFromExactYieldPoint)
{
    std::vector<std::string> events;
    std::mutex mutex;

    {
        FiberJobSystem scheduler(1);

        scheduler.submit([&] {
            {
                std::lock_guard<std::mutex> lock(mutex);
                events.push_back("A1");
            }

            scheduler.yield_current();

            {
                std::lock_guard<std::mutex> lock(mutex);
                events.push_back("A2");
            }
        });

        scheduler.submit([&] {
            std::lock_guard<std::mutex> lock(mutex);
            events.push_back("B");
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        {
            std::lock_guard<std::mutex> lock(mutex);
            ASSERT_EQ(events.size(), 2u);
            EXPECT_EQ(events[0], "A1");
            EXPECT_EQ(events[1], "B");
        }

        scheduler.resume_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    ASSERT_EQ(events.size(), 3u);
    EXPECT_EQ(events[0], "A1");
    EXPECT_EQ(events[1], "B");
    EXPECT_EQ(events[2], "A2");
}

TEST(FiberJobSystemScheduling, OtherWorkRunsWhileFiberIsSuspended)
{
    int counter = 0;
    std::mutex mutex;

    {
        FiberJobSystem scheduler(1);

        scheduler.submit([&] {
            scheduler.yield_current();
        });

        scheduler.submit([&] {
            std::lock_guard<std::mutex> lock(mutex);
            ++counter;
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    EXPECT_EQ(counter, 1);
}
