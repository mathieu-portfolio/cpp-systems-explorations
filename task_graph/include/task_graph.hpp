#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

class JobSystem;

struct TaskId { std::uint32_t value = 0; };

class TaskGraph {
public:
    explicit TaskGraph(std::size_t worker_count);
    ~TaskGraph();

    TaskId add_task(std::function<void()> fn);
    TaskId add_named_task(std::string name, std::function<void()> fn);

    void add_edge(TaskId from, TaskId to);
    void add_edge(const std::string& from, const std::string& to);

    void run();
    void wait();

private:
    struct TaskNode {
        std::string name;
        std::function<void()> fn;
        std::vector<TaskId> outgoing;
    };

    TaskNode& get_task(TaskId id);
    const TaskNode& get_task(TaskId id) const;

    TaskId get_task_id(const std::string& name) const;
    void validate_acyclic() const;

    std::vector<TaskNode> tasks_;
    std::unordered_map<std::string, TaskId> name_to_id_;

    JobSystem* jobs_ = nullptr;
    bool started_ = false;
};
