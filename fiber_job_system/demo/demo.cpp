#include "fiber_job_system.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

namespace
{
constexpr const char* COLOR_RESET  = "\033[0m";
constexpr const char* COLOR_MAIN   = "\033[36m";
constexpr const char* COLOR_WORKER = "\033[33m";
constexpr const char* COLOR_FIBER  = "\033[32m";
constexpr const char* COLOR_STATE  = "\033[35m";

class DemoLog
{
public:
    void print_legend()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        std::cout << "Fiber Job System Demo\n";
        std::cout << "=====================\n\n";
        std::cout << "How to read this output:\n";
        std::cout << "  MAIN   : code outside the scheduler that submits work or resumes suspended fibers\n";
        std::cout << "  WORKER : the worker thread currently executing scheduled work\n";
        std::cout << "  FIBER  : the logical task currently running inside the worker\n";
        std::cout << "  STATE  : scheduler-visible state changes such as yield or resume\n\n";
        std::cout << "Important idea:\n";
        std::cout << "  A fiber can pause itself with yield_current().\n";
        std::cout << "  Later, resume_all() allows that paused fiber to continue from the same point.\n\n";
        std::cout << "Output format:\n";
        std::cout << "[step] TYPE    NAME      message\n\n";
    }

    void line(const char* color,
              const std::string& type,
              const std::string& name,
              const std::string& message)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        std::cout
            << "[" << std::setw(2) << step_++ << "] "
            << color << std::left << std::setw(7) << type << COLOR_RESET << " "
            << std::left << std::setw(9) << name
            << message << '\n';
    }

private:
    std::mutex mutex_;
    int step_ = 1;
};

void pause()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
}

} // namespace

int main()
{
    FiberJobSystem scheduler(1);
    DemoLog log;

    log.print_legend();

    log.line(COLOR_MAIN, "MAIN", "-", "create scheduler with 1 worker");
    log.line(COLOR_MAIN, "MAIN", "-", "submit fiber A");
    scheduler.submit([&] {
        log.line(COLOR_WORKER, "WORKER", "worker-0", "starts executing fiber A");
        log.line(COLOR_FIBER, "FIBER", "fiber-A", "phase 1 begins");
        log.line(COLOR_STATE, "STATE", "fiber-A", "yield_current() called, fiber A becomes suspended");

        scheduler.yield_current();

        log.line(COLOR_WORKER, "WORKER", "worker-0", "continues executing fiber A after resume");
        log.line(COLOR_FIBER, "FIBER", "fiber-A", "phase 2 begins exactly after the previous yield");
        log.line(COLOR_STATE, "STATE", "fiber-A", "yield_current() called again, fiber A becomes suspended");

        scheduler.yield_current();

        log.line(COLOR_WORKER, "WORKER", "worker-0", "continues executing fiber A after second resume");
        log.line(COLOR_FIBER, "FIBER", "fiber-A", "final phase runs");
        log.line(COLOR_FIBER, "FIBER", "fiber-A", "completes");
    });

    log.line(COLOR_MAIN, "MAIN", "-", "submit fiber B");
    scheduler.submit([&] {
        log.line(COLOR_WORKER, "WORKER", "worker-0", "starts executing fiber B");
        log.line(COLOR_FIBER, "FIBER", "fiber-B", "runs while fiber A is suspended");
        log.line(COLOR_FIBER, "FIBER", "fiber-B", "completes");
    });

    pause();

    log.line(COLOR_MAIN, "MAIN", "-", "call resume_all()");
    log.line(COLOR_STATE, "STATE", "-", "all suspended fibers become runnable again");
    scheduler.resume_all();

    pause();

    log.line(COLOR_MAIN, "MAIN", "-", "call resume_all() again");
    log.line(COLOR_STATE, "STATE", "-", "all suspended fibers become runnable again");
    scheduler.resume_all();

    pause();

    log.line(COLOR_MAIN, "MAIN", "-", "demo complete");
    return 0;
}