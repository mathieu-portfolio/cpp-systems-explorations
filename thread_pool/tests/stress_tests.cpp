#include <gtest/gtest.h>

#include "thread_pool.hpp"

#include <atomic>
#include <thread>
#include <vector>

TEST(ThreadPoolStress, ManySmallJobsComplete)
{
  std::atomic<int> counter{0};

  {
    ThreadPool pool(4);

    constexpr int job_count = 1000;
    for (int i = 0; i < job_count; ++i)
    {
      pool.submit([&counter] {
        counter.fetch_add(1, std::memory_order_relaxed);
      });
    }
  }

  EXPECT_EQ(counter.load(std::memory_order_relaxed), 1000);
}

TEST(ThreadPoolStress, ConcurrentSubmittersWorkCorrectly)
{
  std::atomic<int> counter{0};

  {
    ThreadPool pool(4);

    constexpr int submitter_count = 4;
    constexpr int jobs_per_submitter = 250;

    std::vector<std::thread> submitters;
    submitters.reserve(submitter_count);

    for (int i = 0; i < submitter_count; ++i)
    {
      submitters.emplace_back([&pool, &counter, jobs_per_submitter] {
        for (int j = 0; j < jobs_per_submitter; ++j) {
          pool.submit([&counter] {
            counter.fetch_add(1, std::memory_order_relaxed);
          });
        }
      });
    }

    for (auto &t : submitters)
    {
      t.join();
    }
  }

  EXPECT_EQ(counter.load(std::memory_order_relaxed), 4 * 250);
}