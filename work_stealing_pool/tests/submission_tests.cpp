#include <gtest/gtest.h>

#include "work_stealing_pool.hpp"

#include <atomic>

TEST(WorkStealingPoolSubmission, AcceptsValidSubmission)
{
    WorkStealingPool pool(2);
    EXPECT_NO_THROW(pool.submit([] {}));
}

TEST(WorkStealingPoolSubmission, ExecutesSubmittedJobsBeforeDestruction)
{
    std::atomic<int> counter{0};

    {
        WorkStealingPool pool(4);

        for (int i = 0; i < 64; ++i)
        {
            pool.submit([&] {
                counter.fetch_add(1, std::memory_order_relaxed);
            });
        }
    }

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 64);
}

TEST(WorkStealingPoolSubmission, ContinuesAfterThrowingJob)
{
    std::atomic<int> counter{0};

    {
        WorkStealingPool pool(2);

        pool.submit([] {
            throw 42;
        });

        pool.submit([&] {
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 1);
}

TEST(WorkStealingPoolSubmission, HandlesLargerBatchWithPerWorkerQueues)
{
    std::atomic<int> counter{0};

    {
        WorkStealingPool pool(8);

        for (int i = 0; i < 500; ++i)
        {
            pool.submit([&] {
                counter.fetch_add(1, std::memory_order_relaxed);
            });
        }
    }

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 500);
}
