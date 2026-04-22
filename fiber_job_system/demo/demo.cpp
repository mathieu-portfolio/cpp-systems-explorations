#include "fiber_job_system.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

int main()
{
    FiberJobSystem scheduler(1);
    std::mutex cout_mutex;

    auto log = [&](const char* msg) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << msg << '\n';
    };

    scheduler.submit([&] {
        log("[fiber A] part 1");
        log("[fiber A] yielding...");
        scheduler.yield_current();
        log("[fiber A] resumed exactly here");
    });

    scheduler.submit([&] {
        log("[fiber B] runs while A is suspended");
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    log("[main] resuming suspended fibers");
    scheduler.resume_all();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    log("[main] demo complete");
    return 0;
}
