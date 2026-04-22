#include "fiber_job_system.hpp"

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace
{
struct YieldException final : std::exception
{
};
}

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
        FiberState state = FiberState::Runnable;
    };

    std::vector<std::thread> workers;

    std::deque<std::unique_ptr<Fiber>> runnable;
    std::vector<std::unique_ptr<Fiber>> suspended;

    std::mutex mutex;
    std::condition_variable cv;

    bool stopping = false;
};

thread_local void* t_current_impl = nullptr;
thread_local void* t_current_fiber = nullptr;

FiberJobSystem::FiberJobSystem(std::size_t worker_count)
{
    if (worker_count == 0)
    {
        throw std::invalid_argument("FiberJobSystem requires at least one worker");
    }

    impl_ = new Impl{};
    impl_->workers.reserve(worker_count);

    for (std::size_t i = 0; i < worker_count; ++i)
    {
        impl_->workers.emplace_back([this] {
            while (true)
            {
                std::unique_ptr<Impl::Fiber> fiber;

                {
                    std::unique_lock<std::mutex> lock(impl_->mutex);
                    impl_->cv.wait(lock, [this] {
                        return impl_->stopping || !impl_->runnable.empty();
                    });

                    if (impl_->stopping && impl_->runnable.empty())
                    {
                        return;
                    }

                    fiber = std::move(impl_->runnable.front());
                    impl_->runnable.pop_front();
                    fiber->state = Impl::FiberState::Running;
                }

                t_current_impl = impl_;
                t_current_fiber = fiber.get();

                try
                {
                    fiber->fn();
                    fiber->state = Impl::FiberState::Completed;
                }
                catch (const YieldException&)
                {
                    fiber->state = Impl::FiberState::Suspended;

                    std::lock_guard<std::mutex> lock(impl_->mutex);
                    impl_->suspended.push_back(std::move(fiber));
                }

                t_current_fiber = nullptr;
                t_current_impl = nullptr;
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

    {
        std::lock_guard<std::mutex> lock(impl_->mutex);

        if (impl_->stopping)
        {
            throw std::logic_error("submit() cannot be called after shutdown begins");
        }

        auto fiber = std::make_unique<Impl::Fiber>();
        fiber->fn = std::move(job);
        fiber->state = Impl::FiberState::Runnable;
        impl_->runnable.push_back(std::move(fiber));
    }

    impl_->cv.notify_one();
}

void FiberJobSystem::yield_current()
{
    if (t_current_impl != static_cast<void*>(impl_) || t_current_fiber == nullptr)
    {
        throw std::logic_error("yield_current() must be called from a running fiber owned by this scheduler");
    }

    throw YieldException{};
}

void FiberJobSystem::resume_all()
{
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);

        if (impl_->stopping)
        {
            throw std::logic_error("resume_all() cannot be called after shutdown begins");
        }

        for (std::unique_ptr<Impl::Fiber>& fiber : impl_->suspended)
        {
            fiber->state = Impl::FiberState::Runnable;
            impl_->runnable.push_back(std::move(fiber));
        }

        impl_->suspended.clear();
    }

    impl_->cv.notify_all();
}
