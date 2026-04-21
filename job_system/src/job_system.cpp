#include "job_system.hpp"

#include "thread_pool.hpp"

#include <deque>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

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

    jobs_.emplace_back(std::move(fn));
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

    dependent.remaining_dependencies.fetch_add(1, std::memory_order_relaxed);
    prerequisite.dependents.push_back(job);
}

void JobSystem::validate_acyclic() const
{
    std::vector<std::size_t> indegrees;
    indegrees.reserve(jobs_.size());

    for (const JobNode& job : jobs_)
    {
        indegrees.push_back(job.remaining_dependencies.load(std::memory_order_relaxed));
    }

    std::deque<JobId> ready;
    for (std::size_t i = 0; i < indegrees.size(); ++i)
    {
        if (indegrees[i] == 0)
        {
            ready.push_back(JobId{static_cast<std::uint32_t>(i)});
        }
    }

    std::size_t visited = 0;

    while (!ready.empty())
    {
        const JobId current = ready.front();
        ready.pop_front();
        ++visited;

        const JobNode& node = jobs_[static_cast<std::size_t>(current.value)];
        for (JobId dependent_id : node.dependents)
        {
            std::size_t& indegree = indegrees[static_cast<std::size_t>(dependent_id.value)];

            if (indegree == 0)
            {
                throw std::logic_error("Dependency validation encountered an invalid indegree transition");
            }

            indegree -= 1;
            if (indegree == 0)
            {
                ready.push_back(dependent_id);
            }
        }
    }

    if (visited != jobs_.size())
    {
        throw std::logic_error("run() rejected a cyclic dependency graph");
    }
}

void JobSystem::schedule_job(JobId id)
{
    pool_->submit([this, id] {
        JobNode& job = get_job(id);

        try
        {
            job.fn();
        }
        catch (...)
        {
            // Job exceptions are contained within the job system.
            // A throwing job is still treated as completed so that
            // dependents can be unlocked and wait() can make progress.
        }

        complete_job(id);
    });
}

void JobSystem::complete_job(JobId id)
{
    JobNode& completed = get_job(id);

    for (JobId dependent_id : completed.dependents)
    {
        JobNode& dependent = get_job(dependent_id);

        const std::size_t previous =
            dependent.remaining_dependencies.fetch_sub(1, std::memory_order_acq_rel);

        if (previous == 0)
        {
            throw std::logic_error("Dependency counter underflow");
        }

        if (previous == 1)
        {
            schedule_job(dependent_id);
        }
    }

    const std::size_t previous_unfinished =
        unfinished_jobs_.fetch_sub(1, std::memory_order_acq_rel);

    if (previous_unfinished == 0)
    {
        throw std::logic_error("Unfinished job counter underflow");
    }

    if (previous_unfinished == 1)
    {
        std::lock_guard<std::mutex> lock(wait_mutex_);
        wait_cv_.notify_all();
    }
}

void JobSystem::run()
{
    if (started_)
    {
        throw std::logic_error("run() cannot be called more than once");
    }

    validate_acyclic();

    started_ = true;
    unfinished_jobs_.store(jobs_.size(), std::memory_order_release);

    for (std::size_t i = 0; i < jobs_.size(); ++i)
    {
        if (jobs_[i].remaining_dependencies.load(std::memory_order_acquire) == 0)
        {
            schedule_job(JobId{static_cast<std::uint32_t>(i)});
        }
    }
}

void JobSystem::wait()
{
    std::unique_lock<std::mutex> lock(wait_mutex_);
    wait_cv_.wait(lock, [this] {
        return unfinished_jobs_.load(std::memory_order_acquire) == 0;
    });
}
