#include <gtest/gtest.h>

#include "thread_pool.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

TEST(ThreadPoolBasic, ExecutesSubmittedJobs)
{
  std::atomic<int> counter{0};

  {
    ThreadPool pool(4);

    for (int i = 0; i < 20; ++i)
    {
      pool.submit([&counter] {
        counter.fetch_add(1, std::memory_order_relaxed);
      });
    }
  }

  EXPECT_EQ(counter.load(std::memory_order_relaxed), 20);
}

TEST(ThreadPoolBasic, DestructorWaitsForPendingJobsToFinish)
{
  std::atomic<int> counter{0};

  {
    ThreadPool pool(4);

    for (int i = 0; i < 50; ++i)
    {
      pool.submit([&counter] {
        counter.fetch_add(1, std::memory_order_relaxed);
      });
    }
  }

  EXPECT_EQ(counter.load(std::memory_order_relaxed), 50);
}

TEST(ThreadPoolBasic, JobsCanNotifyCompletion)
{
  ThreadPool pool(2);

  std::mutex mutex;
  std::condition_variable cv;
  int completed = 0;
  constexpr int target = 10;

  for (int i = 0; i < target; ++i)
  {
    pool.submit([&] {
      std::lock_guard<std::mutex> lock(mutex);
      ++completed;
      cv.notify_one();
    });
  }

  std::unique_lock<std::mutex> lock(mutex);
  const bool finished = cv.wait_for(lock, std::chrono::seconds(2), [&] {
    return completed == target;
  });

  EXPECT_TRUE(finished);
  EXPECT_EQ(completed, target);
}

TEST(ThreadPoolBasic, MultipleWorkersCanMakeProgress)
{
  std::atomic<int> started{0};
  std::atomic<int> finished{0};

  {
    ThreadPool pool(4);

    for (int i = 0; i < 8; ++i)
    {
      pool.submit([&] {
        started.fetch_add(1, std::memory_order_relaxed);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        finished.fetch_add(1, std::memory_order_relaxed);
      });
    }
  }

  EXPECT_EQ(started.load(std::memory_order_relaxed), 8);
  EXPECT_EQ(finished.load(std::memory_order_relaxed), 8);
}
