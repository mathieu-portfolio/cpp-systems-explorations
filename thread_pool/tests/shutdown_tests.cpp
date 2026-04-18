#include <gtest/gtest.h>

#include "thread_pool.hpp"

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>

TEST(ThreadPoolShutdown, FinishesQueuedJobsBeforeDestructionReturns)
{
  std::atomic<int> counter{0};

  {
    ThreadPool pool(3);

    for (int i = 0; i < 30; ++i)
    {
      pool.submit([&counter] {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        counter.fetch_add(1, std::memory_order_relaxed);
      });
    }
  }

  EXPECT_EQ(counter.load(std::memory_order_relaxed), 30);
}

TEST(ThreadPoolShutdown, HandlesMoreJobsThanWorkers)
{
  std::atomic<int> counter{0};

  {
    ThreadPool pool(2);

    for (int i = 0; i < 100; ++i)
    {
      pool.submit([&counter] {
        counter.fetch_add(1, std::memory_order_relaxed);
      });
    }
  }

  EXPECT_EQ(counter.load(std::memory_order_relaxed), 100);
}

TEST(ThreadPoolShutdown, LongRunningJobsStillCompleteBeforeDestructorReturns)
{
  std::atomic<int> counter{0};

  {
    ThreadPool pool(2);

    for (int i = 0; i < 6; ++i)
    {
      pool.submit([&counter] {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        counter.fetch_add(1, std::memory_order_relaxed);
      });
    }
  }

  EXPECT_EQ(counter.load(std::memory_order_relaxed), 6);
}