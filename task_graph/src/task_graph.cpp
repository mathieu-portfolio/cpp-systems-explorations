#include "task_graph.hpp"

#include "job_system.hpp"

#include <stdexcept>
#include <utility>

TaskGraph::TaskGraph(std::size_t worker_count)
    : jobs_(new JobSystem(worker_count))
{
}

TaskGraph::~TaskGraph()
{
    delete jobs_;
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

void TaskGraph::add_edge(TaskId, TaskId)
{
    throw std::logic_error("add_edge() not implemented yet");
}

void TaskGraph::run()
{
    throw std::logic_error("run() not implemented yet");
}

void TaskGraph::wait()
{
    throw std::logic_error("wait() not implemented yet");
}
