#pragma once
#include <cstddef>
#include <functional>

enum class FiberStepResult
{
    Yield,
    Complete
};

class FiberJobSystem
{
public:
    explicit FiberJobSystem(std::size_t worker_count);
    ~FiberJobSystem();

    FiberJobSystem(const FiberJobSystem&) = delete;
    FiberJobSystem& operator=(const FiberJobSystem&) = delete;

    void submit(std::function<void()> job);
    void submit_resumable(std::function<FiberStepResult()> step);
    void yield_current();
    void resume_all();

private:
    struct Impl;
    Impl* impl_ = nullptr;
};
