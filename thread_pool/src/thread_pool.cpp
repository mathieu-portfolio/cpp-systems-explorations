#include "thread_pool.hpp"
#include <stdexcept>

ThreadPool::ThreadPool(std::size_t thread_count)
{
  for (std::size_t i = 0; i < thread_count; ++i)
  {
    workers_.emplace_back([this] {
      worker_loop();
    });
  }
}

ThreadPool::~ThreadPool()
{
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_ = true;
  }

  cv_.notify_all();

  for (auto& t : workers_)
  {
    if (t.joinable())
    {
      t.join();
    }
  }
}

void ThreadPool::worker_loop()
{
  while (true)
  {
    std::function<void()> job;

    {
      std::unique_lock<std::mutex> lock(mutex_);

      cv_.wait(lock, [this] {
        return stop_ || !jobs_.empty();
      });

      if (stop_ && jobs_.empty())
      {
        return;
      }

      job = std::move(jobs_.front());
      jobs_.pop();
    }

    job();
  }
}

void ThreadPool::submit(std::function<void()> job)
{
  {
    std::lock_guard<std::mutex> lock(mutex_);

    if (stop_)
    {
      throw std::runtime_error("submit() called on stopped ThreadPool");
    }

    jobs_.push(std::move(job));
  }

  cv_.notify_one();
}