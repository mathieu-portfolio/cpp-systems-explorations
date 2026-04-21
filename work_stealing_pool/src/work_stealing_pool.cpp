#include "work_stealing_pool.hpp"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

struct WorkStealingPool::Impl
{
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> jobs;
    std::mutex mutex;
    std::condition_variable cv;
    bool stopping = false;
};

WorkStealingPool::WorkStealingPool(std::size_t worker_count)
{
    if (worker_count == 0)
    {
        throw std::invalid_argument("WorkStealingPool requires at least one worker");
    }

    impl_ = new Impl{};

    impl_->workers.reserve(worker_count);
    for (std::size_t i = 0; i < worker_count; ++i)
    {
        impl_->workers.emplace_back([this] {
            while (true)
            {
                std::function<void()> job;

                {
                    std::unique_lock<std::mutex> lock(impl_->mutex);
                    impl_->cv.wait(lock, [this] {
                        return impl_->stopping || !impl_->jobs.empty();
                    });

                    if (impl_->stopping && impl_->jobs.empty())
                    {
                        return;
                    }

                    job = std::move(impl_->jobs.front());
                    impl_->jobs.pop();
                }

                try
                {
                    job();
                }
                catch (...)
                {
                    // Baseline behavior matches the existing pool design:
                    // keep the worker alive and treat the job as finished.
                }
            }
        });
    }
}

WorkStealingPool::~WorkStealingPool()
{
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        impl_->stopping = true;
    }

    impl_->cv.notify_all();

    for (std::thread& worker : impl_->workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }

    delete impl_;
}

void WorkStealingPool::submit(std::function<void()> job)
{
    if (!job)
    {
        throw std::invalid_argument("submit() requires a valid callable");
    }

    {
        std::lock_guard<std::mutex> lock(impl_->mutex);

        if (impl_->stopping)
        {
            throw std::logic_error("submit() cannot be called after shutdown begins");
        }

        impl_->jobs.push(std::move(job));
    }

    impl_->cv.notify_one();
}
