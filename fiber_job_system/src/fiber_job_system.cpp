#include "fiber_job_system.hpp"

#include <boost/context/continuation.hpp>

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace ctx = boost::context;

thread_local void* t_current_impl = nullptr;
thread_local void* t_current_fiber = nullptr;
thread_local ctx::continuation* t_scheduler_sink = nullptr;

struct FiberJobSystem::Impl
{
    enum class FiberState
    {
        Runnable,
        Running,
        Suspended,
        Completed
    };

    struct Fiber
    {
        std::function<void()> fn;
        ctx::continuation cont;
        ctx::continuation* scheduler_sink = nullptr;
        FiberState state = FiberState::Runnable;
        bool started = false;
        std::size_t owner_worker = std::numeric_limits<std::size_t>::max();
    };

    struct WorkerQueues
    {
        std::deque<std::unique_ptr<Fiber>> resumed;
        std::vector<std::unique_ptr<Fiber>> suspended;
    };

    std::vector<std::thread> workers;
    std::vector<std::unique_ptr<WorkerQueues>> worker_queues;
    std::deque<std::unique_ptr<Fiber>> pending;

    std::mutex mutex;
    std::condition_variable cv;
    bool stopping = false;
};

FiberJobSystem::FiberJobSystem(std::size_t worker_count)
{
    if (worker_count == 0)
    {
        throw std::invalid_argument("FiberJobSystem requires at least one worker");
    }

    impl_ = new Impl{};
    impl_->workers.reserve(worker_count);
    impl_->worker_queues.reserve(worker_count);

    for (std::size_t i = 0; i < worker_count; ++i)
    {
        impl_->worker_queues.push_back(std::make_unique<Impl::WorkerQueues>());
    }

    for (std::size_t worker_index = 0; worker_index < worker_count; ++worker_index)
    {
        impl_->workers.emplace_back([this, worker_index] {
            while (true)
            {
                std::unique_ptr<Impl::Fiber> fiber;

                {
                    std::unique_lock<std::mutex> lock(impl_->mutex);
                    impl_->cv.wait(lock, [this, worker_index] {
                        return impl_->stopping ||
                               !impl_->worker_queues[worker_index]->resumed.empty() ||
                               !impl_->pending.empty();
                    });

                    auto& queues = *impl_->worker_queues[worker_index];

                    if (!queues.resumed.empty())
                    {
                        fiber = std::move(queues.resumed.front());
                        queues.resumed.pop_front();
                    }
                    else if (!impl_->pending.empty())
                    {
                        fiber = std::move(impl_->pending.front());
                        impl_->pending.pop_front();
                    }
                    else if (impl_->stopping)
                    {
                        return;
                    }
                    else
                    {
                        continue;
                    }

                    fiber->state = Impl::FiberState::Running;
                }

                if (!fiber->started)
                {
                    fiber->owner_worker = worker_index;

                    Impl::Fiber* raw_fiber = fiber.get();
                    Impl* raw_impl = impl_;

                    fiber->cont = ctx::callcc([raw_fiber, raw_impl](ctx::continuation&& sink) mutable {
                        raw_fiber->scheduler_sink = &sink;

                        // Hand control back immediately so the fiber body starts on first worker resume.
                        sink = std::move(sink).resume();

                        t_current_impl = raw_impl;
                        t_current_fiber = raw_fiber;
                        t_scheduler_sink = raw_fiber->scheduler_sink;

                        raw_fiber->fn();
                        raw_fiber->state = Impl::FiberState::Completed;

                        t_scheduler_sink = nullptr;
                        t_current_fiber = nullptr;
                        t_current_impl = nullptr;

                        return std::move(sink);
                    });

                    fiber->started = true;
                }

                t_current_impl = impl_;
                t_current_fiber = fiber.get();
                t_scheduler_sink = fiber->scheduler_sink;

                fiber->cont = std::move(fiber->cont).resume();

                t_scheduler_sink = nullptr;
                t_current_fiber = nullptr;
                t_current_impl = nullptr;

                if (fiber->state == Impl::FiberState::Suspended)
                {
                    std::lock_guard<std::mutex> lock(impl_->mutex);
                    impl_->worker_queues[worker_index]->suspended.push_back(std::move(fiber));
                }
                else if (fiber->state == Impl::FiberState::Completed)
                {
                    // Continuation storage is released when the fiber is destroyed.
                }
                else
                {
                    throw std::logic_error("Fiber returned to scheduler in invalid state");
                }
            }
        });
    }
}

FiberJobSystem::~FiberJobSystem()
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

void FiberJobSystem::submit(std::function<void()> job)
{
    if (!job)
    {
        throw std::invalid_argument("submit() requires a valid callable");
    }

    auto fiber = std::make_unique<Impl::Fiber>();
    fiber->fn = std::move(job);
    fiber->state = Impl::FiberState::Runnable;

    {
        std::lock_guard<std::mutex> lock(impl_->mutex);

        if (impl_->stopping)
        {
            throw std::logic_error("submit() cannot be called after shutdown begins");
        }

        impl_->pending.push_back(std::move(fiber));
    }

    impl_->cv.notify_one();
}

void FiberJobSystem::yield_current()
{
    if (t_current_impl != static_cast<void*>(impl_) || t_current_fiber == nullptr || t_scheduler_sink == nullptr)
    {
        throw std::logic_error("yield_current() must be called from a running fiber owned by this scheduler");
    }

    auto* fiber = static_cast<Impl::Fiber*>(t_current_fiber);
    fiber->state = Impl::FiberState::Suspended;

    *t_scheduler_sink = std::move(*t_scheduler_sink).resume();

    fiber->state = Impl::FiberState::Running;
}

void FiberJobSystem::resume_all()
{
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);

        if (impl_->stopping)
        {
            throw std::logic_error("resume_all() cannot be called after shutdown begins");
        }

        for (std::size_t worker_index = 0; worker_index < impl_->worker_queues.size(); ++worker_index)
        {
            auto& queues = *impl_->worker_queues[worker_index];

            for (std::unique_ptr<Impl::Fiber>& fiber : queues.suspended)
            {
                fiber->state = Impl::FiberState::Runnable;
                queues.resumed.push_back(std::move(fiber));
            }

            queues.suspended.clear();
        }
    }

    impl_->cv.notify_all();
}