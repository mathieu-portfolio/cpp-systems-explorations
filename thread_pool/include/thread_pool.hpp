#pragma once

#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

[[noreturn]] inline void thread_pool_contract_fail(
  const char *file,
  int line,
  const char *expression,
  const char *message)
{
  std::fprintf(
    stderr,
    "thread_pool contract violation at %s:%d\n"
    "  expression: %s\n"
    "  message: %s\n",
    file,
    line,
    expression,
    message);
  std::abort();
}

#define THREAD_POOL_CHECK(expr, message)                             \
  do                                                                 \
  {                                                                  \
    if (!(expr))                                                     \
    {                                                                \
      thread_pool_contract_fail(__FILE__, __LINE__, #expr, message); \
    }                                                                \
  } while (false)

class ThreadPool
{
private:
  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> jobs_;

  mutable std::mutex mutex_;
  std::condition_variable cv_;
  bool stop_ = false;

  void worker_loop();
  void assert_locked_invariants() const;

public:
  explicit ThreadPool(std::size_t thread_count);
  ~ThreadPool();

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool &operator=(ThreadPool &&) = delete;

  void submit(std::function<void()> job);
};