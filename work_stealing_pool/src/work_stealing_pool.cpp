#include "work_stealing_pool.hpp"

#include <stdexcept>

struct WorkStealingPool::Impl
{
};

WorkStealingPool::WorkStealingPool(std::size_t worker_count)
{
    if (worker_count == 0)
    {
        throw std::invalid_argument("WorkStealingPool requires at least one worker");
    }

    impl_ = new Impl{};
}

WorkStealingPool::~WorkStealingPool()
{
    delete impl_;
}

void WorkStealingPool::submit(std::function<void()> job)
{
    if (!job)
    {
        throw std::invalid_argument("submit() requires a valid callable");
    }

    throw std::logic_error("submit() not implemented yet");
}
