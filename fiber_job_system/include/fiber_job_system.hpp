#pragma once

#include <cstddef>
#include <functional>

class FiberJobSystem
{
public:
    explicit FiberJobSystem(std::size_t worker_count);
    ~FiberJobSystem();

    FiberJobSystem(const FiberJobSystem&) = delete;
    FiberJobSystem& operator=(const FiberJobSystem&) = delete;
    FiberJobSystem(FiberJobSystem&&) = delete;
    FiberJobSystem& operator=(FiberJobSystem&&) = delete;

    void submit(std::function<void()> job);

    // Suspends the currently running fiber and returns control to the worker scheduler.
    void yield_current();

    // Moves all suspended fibers back to runnable work.
    void resume_all();

private:
    struct Impl;
    Impl* impl_ = nullptr;
};
