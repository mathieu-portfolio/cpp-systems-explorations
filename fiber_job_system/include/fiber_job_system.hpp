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

    // Planned cooperative scheduling API for the first real implementation step.
    void yield_current();

private:
    struct Impl;
    Impl* impl_ = nullptr;
};
