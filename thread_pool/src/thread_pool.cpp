#include "thread_pool.hpp"
#include <stdexcept>

ThreadPool::ThreadPool(std::size_t thread_count)
{
  if (thread_count == 0)
  {
    throw std::invalid_argument("ThreadPool requires at least one worker thread");
  }

  workers_.reserve(thread_count);

  try
  {
    for (std::size_t i = 0; i < thread_count; ++i)
    {
      workers_.emplace_back([this] { worker_loop(); });
    }
  }
  catch (...)
  {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stop_ = true;
    }

    cv_.notify_all();

    for (auto &worker : workers_)
    {
      if (worker.joinable())
      {
        worker.join();
      }
    }

    throw;
  }
}

ThreadPool::~ThreadPool()
{
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_ = true;
    assert_locked_invariants();
  }

  cv_.notify_all();

  for (auto &worker : workers_)
  {
    if (worker.joinable())
    {
      worker.join();
    }
  }
}

void ThreadPool::submit(std::function<void()> job)
{
  if (!job)
  {
    throw std::invalid_argument("submit() requires a valid job");
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);

    assert_locked_invariants();

    if (stop_)
    {
      throw std::runtime_error("submit() called on stopped ThreadPool");
    }

    jobs_.push(std::move(job));

    assert_locked_invariants();
  }

  cv_.notify_one();
}

void ThreadPool::worker_loop()
{
  while (true)
  {
    std::function<void()> job;

    {
      std::unique_lock<std::mutex> lock(mutex_);

      cv_.wait(lock, [this] { return stop_ || !jobs_.empty(); });

      assert_locked_invariants();

      if (stop_ && jobs_.empty())
      {
        return;
      }

      THREAD_POOL_CHECK(!jobs_.empty(), "worker must not pop from an empty queue");

      job = std::move(jobs_.front());
      jobs_.pop();

      assert_locked_invariants();
    }

    try
    {
      job();
    }
    catch (...)
    {
      // Exception policy:
      // job exceptions are contained within the pool so that
      // one failing job does not terminate the worker thread
      // or the entire pool.
    }
  }
}

void ThreadPool::assert_locked_invariants() const
{
  THREAD_POOL_CHECK(
    workers_.size() > 0,
    "ThreadPool must own at least one worker thread");
}