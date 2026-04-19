#include <gtest/gtest.h>

#include "thread_pool.hpp"

TEST(ThreadPoolContract, RejectsZeroWorkerThreads)
{
  EXPECT_THROW(
    {
      ThreadPool pool(0);
    },
    std::invalid_argument);
}

TEST(ThreadPoolContract, RejectsEmptyJobSubmission)
{
  ThreadPool pool(2);

  EXPECT_THROW(
    {
      pool.submit(std::function<void()>{});
    },
    std::invalid_argument);
}