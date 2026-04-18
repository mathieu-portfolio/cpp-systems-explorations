#include "thread_pool.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

int main()
{
  std::mutex cout_mutex;

  {
    ThreadPool pool(3);

    {
      std::lock_guard<std::mutex> lock(cout_mutex);
      std::cout << "Submitting 8 jobs to a pool with 3 workers...\n";
    }

    for (int i = 0; i < 8; ++i)
    {
      pool.submit([i, &cout_mutex] {
        {
          std::lock_guard<std::mutex> lock(cout_mutex);
          std::cout
              << "[job " << i << "] started on thread "
              << std::this_thread::get_id() << '\n';
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(150));

        {
          std::lock_guard<std::mutex> lock(cout_mutex);
          std::cout
              << "[job " << i << "] finished on thread "
              << std::this_thread::get_id() << '\n';
        }
      });
    }

    {
      std::lock_guard<std::mutex> lock(cout_mutex);
      std::cout << "All jobs submitted. Leaving scope will destroy the pool.\n";
      std::cout << "Destructor should wait until queued jobs finish.\n";
    }
  }

  std::cout << "Pool destroyed. All worker threads joined cleanly.\n";
  return 0;
}