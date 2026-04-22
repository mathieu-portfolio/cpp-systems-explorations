#include "fiber_job_system.hpp"
#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <stdexcept>
#include <vector>

namespace { struct YieldException {}; }

struct FiberJobSystem::Impl
{
    struct Fiber
    {
        std::function<void()> one_shot;
        std::function<FiberStepResult()> step;
        bool resumable = false;
    };

    std::deque<std::unique_ptr<Fiber>> runnable;
    std::vector<std::unique_ptr<Fiber>> suspended;

    std::mutex m;
    std::condition_variable cv;
    std::vector<std::thread> workers;
    bool stop = false;
};

thread_local void* t_impl = nullptr;
thread_local void* t_fiber = nullptr;

FiberJobSystem::FiberJobSystem(std::size_t n)
{
    if (n == 0) throw std::invalid_argument("workers");

    impl_ = new Impl{};
    for (size_t i = 0; i < n; ++i)
    {
        impl_->workers.emplace_back([this] {
            while (true)
            {
                std::unique_ptr<Impl::Fiber> f;

                {
                    std::unique_lock lock(impl_->m);
                    impl_->cv.wait(lock, [this]{
                        return impl_->stop || !impl_->runnable.empty();
                    });

                    if (impl_->stop && impl_->runnable.empty()) return;

                    f = std::move(impl_->runnable.front());
                    impl_->runnable.pop_front();
                }

                t_impl = impl_;
                t_fiber = f.get();

                try
                {
                    if (f->resumable)
                    {
                        if (f->step() == FiberStepResult::Yield)
                        {
                            std::lock_guard lock(impl_->m);
                            impl_->suspended.push_back(std::move(f));
                        }
                    }
                    else
                    {
                        f->one_shot();
                    }
                }
                catch (YieldException&)
                {
                    std::lock_guard lock(impl_->m);
                    impl_->suspended.push_back(std::move(f));
                }

                t_impl = nullptr;
                t_fiber = nullptr;
            }
        });
    }
}

FiberJobSystem::~FiberJobSystem()
{
    {
        std::lock_guard lock(impl_->m);
        impl_->stop = true;
    }
    impl_->cv.notify_all();
    for (auto& w : impl_->workers) w.join();
    delete impl_;
}

void FiberJobSystem::submit(std::function<void()> job)
{
    if (!job) throw std::invalid_argument("job");

    auto f = std::make_unique<Impl::Fiber>();
    f->one_shot = std::move(job);

    {
        std::lock_guard lock(impl_->m);
        impl_->runnable.push_back(std::move(f));
    }
    impl_->cv.notify_one();
}

void FiberJobSystem::submit_resumable(std::function<FiberStepResult()> step)
{
    if (!step) throw std::invalid_argument("step");

    auto f = std::make_unique<Impl::Fiber>();
    f->step = std::move(step);
    f->resumable = true;

    {
        std::lock_guard lock(impl_->m);
        impl_->runnable.push_back(std::move(f));
    }
    impl_->cv.notify_one();
}

void FiberJobSystem::yield_current()
{
    if (t_impl != impl_ || t_fiber == nullptr)
        throw std::logic_error("yield outside");

    throw YieldException{};
}

void FiberJobSystem::resume_all()
{
    {
        std::lock_guard lock(impl_->m);
        for (auto& f : impl_->suspended)
            impl_->runnable.push_back(std::move(f));
        impl_->suspended.clear();
    }
    impl_->cv.notify_all();
}
