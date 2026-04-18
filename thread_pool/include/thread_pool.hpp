#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> jobs_;

    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_ = false;

    void worker_loop();

public:
    explicit ThreadPool(std::size_t thread_count);
    ~ThreadPool();

    void submit(std::function<void()> job);
};