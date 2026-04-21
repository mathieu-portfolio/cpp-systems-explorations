#include "job_system.hpp"

#include "thread_pool.hpp"

#include <memory>
#include <stdexcept>

struct JobSystem::JobNode
{
    std::function<void()> fn;
};

JobSystem::JobSystem(std::size_t worker_count)
    : pool_(new ThreadPool(worker_count))
{
}

JobSystem::~JobSystem()
{
    delete pool_;
}

JobId JobSystem::create_job(std::function<void()> fn)
{
    if (!fn)
    {
        throw std::invalid_argument("create_job() requires a valid callable");
    }

    jobs_.push_back(JobNode{std::move(fn)});
    return JobId{static_cast<std::uint32_t>(jobs_.size() - 1)};
}

void JobSystem::add_dependency(JobId, JobId)
{
    throw std::logic_error("add_dependency() not implemented yet");
}

void JobSystem::run()
{
    throw std::logic_error("run() not implemented yet");
}

void JobSystem::wait()
{
    throw std::logic_error("wait() not implemented yet");
}
