#pragma once

#include <cstddef>
#include <functional>
#include <memory>

enum class FiberStepResult
{
    Yield,
    Complete
};

class FiberTask
{
public:
    virtual ~FiberTask() = default;
    virtual FiberStepResult run() = 0;
};

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
    void submit_resumable(std::function<FiberStepResult()> step);
    void submit_task(std::unique_ptr<FiberTask> task);

    void yield_current();
    void resume_all();

private:
    struct Impl;
    Impl* impl_ = nullptr;
};
