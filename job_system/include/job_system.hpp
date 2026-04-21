#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <vector>

class ThreadPool;

struct JobId
{
    std::uint32_t value = 0;
};

class JobSystem
{
public:
    explicit JobSystem(std::size_t worker_count);
    ~JobSystem();

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;
    JobSystem(JobSystem&&) = delete;
    JobSystem& operator=(JobSystem&&) = delete;

    JobId create_job(std::function<void()> fn);
    void add_dependency(JobId job, JobId depends_on);

    void run();
    void wait();

private:
    struct JobNode
    {
        std::function<void()> fn;
        std::atomic<std::size_t> remaining_dependencies{0};
        std::vector<JobId> dependents;

        JobNode() = default;

        explicit JobNode(std::function<void()> f)
            : fn(std::move(f))
        {
        }

        JobNode(const JobNode&) = delete;
        JobNode& operator=(const JobNode&) = delete;

        JobNode(JobNode&& other) noexcept
            : fn(std::move(other.fn)),
              remaining_dependencies(other.remaining_dependencies.load(std::memory_order_relaxed)),
              dependents(std::move(other.dependents))
        {
        }

        JobNode& operator=(JobNode&& other) noexcept
        {
            if (this != &other)
            {
                fn = std::move(other.fn);
                remaining_dependencies.store(
                    other.remaining_dependencies.load(std::memory_order_relaxed),
                    std::memory_order_relaxed);
                dependents = std::move(other.dependents);
            }

            return *this;
        }
    };

    JobNode& get_job(JobId id);
    const JobNode& get_job(JobId id) const;

    void validate_acyclic() const;
    void schedule_job(JobId id);
    void complete_job(JobId id);

    std::vector<JobNode> jobs_;
    ThreadPool* pool_ = nullptr;
    bool started_ = false;

    std::atomic<std::size_t> unfinished_jobs_{0};
    std::mutex wait_mutex_;
    std::condition_variable wait_cv_;
};
