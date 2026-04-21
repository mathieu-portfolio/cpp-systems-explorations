#include "task_graph.hpp"

#include "job_system.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

TaskGraph::TaskGraph(std::size_t worker_count)
    : jobs_(new JobSystem(worker_count))
{
}

TaskGraph::~TaskGraph()
{
    delete jobs_;
}

TaskGraph::TaskNode& TaskGraph::get_task(TaskId id)
{
    if (id.value >= tasks_.size())
    {
        throw std::out_of_range("TaskId is out of range");
    }

    return tasks_[static_cast<std::size_t>(id.value)];
}

const TaskGraph::TaskNode& TaskGraph::get_task(TaskId id) const
{
    if (id.value >= tasks_.size())
    {
        throw std::out_of_range("TaskId is out of range");
    }

    return tasks_[static_cast<std::size_t>(id.value)];
}

TaskId TaskGraph::add_task(std::function<void()> fn)
{
    if (!fn)
    {
        throw std::invalid_argument("add_task() requires a valid callable");
    }

    if (started_)
    {
        throw std::logic_error("add_task() cannot be called after run()");
    }

    tasks_.push_back(TaskNode{"", std::move(fn), {}});
    return TaskId{static_cast<std::uint32_t>(tasks_.size() - 1)};
}

TaskId TaskGraph::add_named_task(std::string name, std::function<void()> fn)
{
    if (!fn)
    {
        throw std::invalid_argument("add_named_task() requires a valid callable");
    }

    if (started_)
    {
        throw std::logic_error("add_named_task() cannot be called after run()");
    }

    tasks_.push_back(TaskNode{std::move(name), std::move(fn), {}});
    return TaskId{static_cast<std::uint32_t>(tasks_.size() - 1)};
}

void TaskGraph::add_edge(TaskId from, TaskId to)
{
    if (started_)
    {
        throw std::logic_error("add_edge() cannot be called after run()");
    }

    if (from.value == to.value)
    {
        throw std::invalid_argument("A task cannot depend on itself");
    }

    TaskNode& source = get_task(from);
    get_task(to);

    source.outgoing.push_back(to);
}

void TaskGraph::run()
{
    if (started_)
    {
        throw std::logic_error("run() cannot be called more than once");
    }

    started_ = true;

    std::vector<JobId> job_ids;
    job_ids.reserve(tasks_.size());

    for (TaskNode& task : tasks_)
    {
        job_ids.push_back(jobs_->create_job(task.fn));
    }

    for (std::size_t from = 0; from < tasks_.size(); ++from)
    {
        for (TaskId to : tasks_[from].outgoing)
        {
            jobs_->add_dependency(
                job_ids[static_cast<std::size_t>(to.value)],
                job_ids[from]);
        }
    }

    jobs_->run();
}

void TaskGraph::wait()
{
    jobs_->wait();
}
