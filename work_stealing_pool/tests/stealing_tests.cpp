#include <gtest/gtest.h>

#include "work_stealing_pool.hpp"

#include <atomic>

TEST(WorkStealingPoolStealing, CompletesLargeBatchWithStealingEnabled)
{
    std::atomic<int> counter{0};

    {
        WorkStealingPool pool(8);

        for (int i = 0; i < 1000; ++i)
        {
            pool.submit([&] {
                counter.fetch_add(1, std::memory_order_relaxed);
            });
        }
    }

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 1000);
}

TEST(WorkStealingPoolStealing, ContinuesToDrainWorkDuringShutdown)
{
    std::atomic<int> counter{0};

    {
        WorkStealingPool pool(4);

        for (int i = 0; i < 128; ++i)
        {
            pool.submit([&] {
                counter.fetch_add(1, std::memory_order_relaxed);
            });
        }
    }

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 128);
}
