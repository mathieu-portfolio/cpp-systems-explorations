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

    bool try_pop_local(std::size_t worker_index, std::function<void()>& job)
    {
        auto& state = *worker_states[worker_index];
        std::lock_guard<std::mutex> lock(state.mutex);

        if (state.jobs.empty())
        {
            return false;
        }

        job = std::move(state.jobs.front());
        state.jobs.pop_front();
        return true;
    }

    bool try_steal(std::size_t thief_index, std::function<void()>& job)
    {
        const std::size_t worker_count = worker_states.size();

        for (std::size_t offset = 1; offset < worker_count; ++offset)
        {
            const std::size_t victim_index = (thief_index + offset) % worker_count;
            auto& victim = *worker_states[victim_index];

            std::lock_guard<std::mutex> lock(victim.mutex);

            if (victim.jobs.empty())
            {
                continue;
            }

            job = std::move(victim.jobs.back());
            victim.jobs.pop_back();
            return true;
        }

        return false;
    }

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

                if (impl_->try_pop_local(worker_index, job) || impl_->try_steal(worker_index, job))
                {
                    try
                    {
                        job();
                    }
                    catch (...)
                    {
                        // Keep the worker alive and treat the job as finished.
                    }

                    continue;
                }

                std::unique_lock<std::mutex> wake_lock(impl_->wake_mutex);
                impl_->wake_cv.wait(wake_lock, [this] {
                    return impl_->stopping || impl_->has_any_work();
                });

                if (impl_->stopping)
                {
                    std::function<void()> remaining_job;
                    if (!impl_->try_pop_local(worker_index, remaining_job) &&
                        !impl_->try_steal(worker_index, remaining_job))
                    {
                        return;
                    }

                    wake_lock.unlock();

                    try
                    {
                        remaining_job();
                    }
                    catch (...)
                    {
                        // Keep shutdown draining semantics simple and robust.
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
