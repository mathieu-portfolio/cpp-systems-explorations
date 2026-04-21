#include "task_graph.hpp"
#include "job_system.hpp"
#include <stdexcept>
#include <vector>
#include <deque>

TaskGraph::TaskGraph(std::size_t w) : jobs_(new JobSystem(w)) {}
TaskGraph::~TaskGraph() { delete jobs_; }

TaskGraph::TaskNode& TaskGraph::get_task(TaskId id) {
    if (id.value >= tasks_.size()) throw std::out_of_range("TaskId out of range");
    return tasks_[id.value];
}
const TaskGraph::TaskNode& TaskGraph::get_task(TaskId id) const {
    if (id.value >= tasks_.size()) throw std::out_of_range("TaskId out of range");
    return tasks_[id.value];
}

TaskId TaskGraph::add_task(std::function<void()> fn) {
    if (!fn) throw std::invalid_argument("invalid fn");
    if (started_) throw std::logic_error("after run");
    tasks_.push_back({"", std::move(fn), {}});
    return TaskId{(uint32_t)tasks_.size()-1};
}

TaskId TaskGraph::add_named_task(std::string n, std::function<void()> fn) {
    if (!fn) throw std::invalid_argument("invalid fn");
    if (started_) throw std::logic_error("after run");
    tasks_.push_back({std::move(n), std::move(fn), {}});
    return TaskId{(uint32_t)tasks_.size()-1};
}

void TaskGraph::add_edge(TaskId from, TaskId to) {
    if (started_) throw std::logic_error("after run");
    if (from.value == to.value) throw std::invalid_argument("self edge");
    auto& src = get_task(from);
    get_task(to);

    for (auto& e : src.outgoing)
        if (e.value == to.value)
            throw std::logic_error("duplicate edge");

    src.outgoing.push_back(to);
}

void TaskGraph::validate_acyclic() const {
    std::vector<size_t> indeg(tasks_.size(),0);
    for (size_t i=0;i<tasks_.size();++i)
        for (auto t: tasks_[i].outgoing)
            indeg[t.value]++;

    std::deque<size_t> q;
    for (size_t i=0;i<indeg.size();++i)
        if (indeg[i]==0) q.push_back(i);

    size_t visited=0;
    while(!q.empty()){
        auto cur=q.front(); q.pop_front(); visited++;
        for(auto t: tasks_[cur].outgoing){
            if(--indeg[t.value]==0) q.push_back(t.value);
        }
    }
    if (visited != tasks_.size())
        throw std::logic_error("cyclic task graph");
}

void TaskGraph::run() {
    if (started_) throw std::logic_error("run twice");

    validate_acyclic();
    started_ = true;

    std::vector<JobId> ids;
    for (auto& t: tasks_)
        ids.push_back(jobs_->create_job(t.fn));

    for (size_t i=0;i<tasks_.size();++i)
        for (auto to: tasks_[i].outgoing)
            jobs_->add_dependency(ids[to.value], ids[i]);

    jobs_->run();
}

void TaskGraph::wait() { jobs_->wait(); }
