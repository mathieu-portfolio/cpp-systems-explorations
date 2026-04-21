#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class JobSystem;

struct TaskId
{
    std::uint32_t value = 0;
};

class TaskGraph
{
public:
    explicit TaskGraph(std::size_t worker_count);
    ~TaskGraph();

    TaskGraph(const TaskGraph&) = delete;
    TaskGraph& operator=(const TaskGraph&) = delete;
    TaskGraph(TaskGraph&&) = delete;
    TaskGraph& operator=(TaskGraph&&) = delete;

    TaskId add_task(std::function<void()> fn);
    TaskId add_named_task(std::string name, std::function<void()> fn);
    void add_edge(TaskId from, TaskId to);

    void run();
    void wait();

private:
    struct TaskNode
    {
        std::string name;
        std::function<void()> fn;
        std::vector<TaskId> outgoing;
    };

    std::vector<TaskNode> tasks_;
    JobSystem* jobs_ = nullptr;
    bool started_ = false;
};
