#include "fiber_job_system.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

namespace
{
class ExampleTask final : public FiberTask
{
public:
    explicit ExampleTask(std::mutex& cout_mutex)
        : cout_mutex_(cout_mutex)
    {
    }

    FiberStepResult run() override
    {
        std::lock_guard<std::mutex> lock(cout_mutex_);

        if (step_ == 0)
        {
            std::cout << "[task] phase 1\n";
            ++step_;
            return FiberStepResult::Yield;
        }

        std::cout << "[task] phase 2\n";
        return FiberStepResult::Complete;
    }

private:
    std::mutex& cout_mutex_;
    int step_ = 0;
};
}

int main()
{
    FiberJobSystem scheduler(2);
    std::mutex cout_mutex;

    scheduler.submit_task(std::make_unique<ExampleTask>(cout_mutex));

    scheduler.submit([&] {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[one-shot] runs while task is suspended\n";
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[main] resuming all suspended fibers\n";
    }
    scheduler.resume_all();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[main] demo complete\n";
    }

    return 0;
}
