#include "fiber_job_system.hpp"

#include <stdexcept>

struct FiberJobSystem::Impl
{
};

FiberJobSystem::FiberJobSystem(std::size_t worker_count)
{
    if (worker_count == 0)
    {
        throw std::invalid_argument("FiberJobSystem requires at least one worker");
    }

    impl_ = new Impl{};
}

FiberJobSystem::~FiberJobSystem()
{
    delete impl_;
}

void FiberJobSystem::submit(std::function<void()> job)
{
    if (!job)
    {
        throw std::invalid_argument("submit() requires a valid callable");
    }

    throw std::logic_error("submit() not implemented yet");
}

void FiberJobSystem::yield_current()
{
    throw std::logic_error("yield_current() not implemented yet");
}
