#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
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
        std::size_t remaining_dependencies = 0;
        std::vector<JobId> dependents;
    };

    JobNode& get_job(JobId id);
    const JobNode& get_job(JobId id) const;

    std::vector<JobNode> jobs_;
    ThreadPool* pool_ = nullptr;
    bool started_ = false;
};
