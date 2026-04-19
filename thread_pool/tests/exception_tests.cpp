#include <gtest/gtest.h>

#include "thread_pool.hpp"

#include <atomic>
#include <stdexcept>

TEST(ThreadPoolExceptions, ThrowingJobDoesNotKillPool) {
    std::atomic<int> counter{0};

    {
        ThreadPool pool(2);

        pool.submit([] {
            throw std::runtime_error("job failure");
        });

        for (int i = 0; i < 10; ++i) {
            pool.submit([&counter] {
                counter.fetch_add(1, std::memory_order_relaxed);
            });
        }
    }

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 10);
}