#include "work_stealing_pool.hpp"

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

struct WorkStealingPool::Impl
{
    struct WorkerState
    {
        std::mutex mutex;
        std::deque<std::function<void()>> jobs;
    };

    std::vector<std::thread> workers;
    std::vector<std::unique_ptr<WorkerState>> worker_states;

    std::mutex wake_mutex;
    std::condition_variable wake_cv;

    bool stopping = false;
    std::size_t next_worker = 0;

    bool has_any_work() const
    {
        for (const auto& state_ptr : worker_states)
        {
            auto& state = *state_ptr;
            std::lock_guard<std::mutex> lock(state.mutex);
            if (!state.jobs.empty())
            {
                return true;
            }
        }
        return false;
    }
};

WorkStealingPool::WorkStealingPool(std::size_t worker_count)
{
    if (worker_count == 0)
    {
        throw std::invalid_argument("WorkStealingPool requires at least one worker");
    }

    impl_ = new Impl{};
    impl_->worker_states.reserve(worker_count);
    impl_->workers.reserve(worker_count);

    for (std::size_t i = 0; i < worker_count; ++i)
    {
        impl_->worker_states.push_back(std::make_unique<Impl::WorkerState>());
    }

    for (std::size_t worker_index = 0; worker_index < worker_count; ++worker_index)
    {
        impl_->workers.emplace_back([this, worker_index] {
            while (true)
            {
                std::function<void()> job;

                // 1. Try local queue first
                {
                    auto& state = *impl_->worker_states[worker_index];
                    std::lock_guard<std::mutex> lock(state.mutex);

                    if (!state.jobs.empty())
                    {
                        job = std::move(state.jobs.front());
                        state.jobs.pop_front();
                    }
                }

                if (job)
                {
                    try { job(); }
                    catch (...) { /* swallow */ }
                    continue;
                }

                // 2. Sleep until shutdown or work appears somewhere
                std::unique_lock<std::mutex> wake_lock(impl_->wake_mutex);
                impl_->wake_cv.wait(wake_lock, [this] {
                    return impl_->stopping || impl_->has_any_work();
                });

                // 3. Shutdown logic
                if (impl_->stopping)
                {
                    bool has_local_work = false;
                    {
                        auto& state = *impl_->worker_states[worker_index];
                        std::lock_guard<std::mutex> lock(state.mutex);
                        has_local_work = !state.jobs.empty();
                    }

                    if (!has_local_work)
                    {
                        return;
                    }
                }
            }
        });
    }
}

WorkStealingPool::~WorkStealingPool()
{
    {
        std::lock_guard<std::mutex> lock(impl_->wake_mutex);
        impl_->stopping = true;
    }

    impl_->wake_cv.notify_all();

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

    std::size_t target_worker = 0;

    {
        std::lock_guard<std::mutex> lock(impl_->wake_mutex);

        if (impl_->stopping)
        {
            throw std::logic_error("submit() cannot be called after shutdown begins");
        }

        target_worker = impl_->next_worker % impl_->worker_states.size();
        ++impl_->next_worker;
    }

    {
        auto& state = *impl_->worker_states[target_worker];
        std::lock_guard<std::mutex> lock(state.mutex);
        state.jobs.push_back(std::move(job));
    }

    impl_->wake_cv.notify_all();
}