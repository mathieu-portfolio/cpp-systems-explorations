#include "job_system.hpp"

#include "thread_pool.hpp"

#include <memory>
#include <stdexcept>

JobSystem::JobSystem(std::size_t worker_count)
    : pool_(new ThreadPool(worker_count))
{
}

JobSystem::~JobSystem()
{
    delete pool_;
}

JobSystem::JobNode& JobSystem::get_job(JobId id)
{
    if (id.value >= jobs_.size())
    {
        throw std::out_of_range("JobId is out of range");
    }

    return jobs_[static_cast<std::size_t>(id.value)];
}

const JobSystem::JobNode& JobSystem::get_job(JobId id) const
{
    if (id.value >= jobs_.size())
    {
        throw std::out_of_range("JobId is out of range");
    }

    return jobs_[static_cast<std::size_t>(id.value)];
}

JobId JobSystem::create_job(std::function<void()> fn)
{
    if (!fn)
    {
        throw std::invalid_argument("create_job() requires a valid callable");
    }

    if (started_)
    {
        throw std::logic_error("create_job() cannot be called after run()");
    }

    jobs_.push_back(JobNode{std::move(fn), 0, {}});
    return JobId{static_cast<std::uint32_t>(jobs_.size() - 1)};
}

void JobSystem::add_dependency(JobId job, JobId depends_on)
{
    if (started_)
    {
        throw std::logic_error("add_dependency() cannot be called after run()");
    }

    if (job.value == depends_on.value)
    {
        throw std::invalid_argument("A job cannot depend on itself");
    }

    JobNode& dependent = get_job(job);
    JobNode& prerequisite = get_job(depends_on);

    dependent.remaining_dependencies += 1;
    prerequisite.dependents.push_back(job);
}

void JobSystem::run()
{
    started_ = true;
    throw std::logic_error("run() not implemented yet");
}

void JobSystem::wait()
{
    throw std::logic_error("wait() not implemented yet");
}
