#pragma once

#include <cstddef>
#include <functional>

class WorkStealingPool
{
public:
    explicit WorkStealingPool(std::size_t worker_count);
    ~WorkStealingPool();

    WorkStealingPool(const WorkStealingPool&) = delete;
    WorkStealingPool& operator=(const WorkStealingPool&) = delete;
    WorkStealingPool(WorkStealingPool&&) = delete;
    WorkStealingPool& operator=(WorkStealingPool&&) = delete;

    void submit(std::function<void()> job);

private:
    struct Impl;
    Impl* impl_ = nullptr;
};
