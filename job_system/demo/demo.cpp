#include "job_system.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

int main()
{
    std::mutex cout_mutex;

    auto log = [&](const char* msg) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << msg << '\n';
    };

    std::cout << "Job graph:\n";
    std::cout << "    A\n";
    std::cout << "   / \\\n";
    std::cout << "  C   D\n";
    std::cout << "   \\ /\n";
    std::cout << "    B (independent)\n\n";

    JobSystem jobs(4);

    const JobId a = jobs.create_job([&] {
        log("[A] start");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        log("[A] done");
    });

    const JobId b = jobs.create_job([&] {
        log("[B] start (independent)");
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        log("[B] done");
    });

    const JobId c = jobs.create_job([&] {
        log("[C] start (after A)");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        log("[C] done");
    });

    const JobId d = jobs.create_job([&] {
        log("[D] start (after A and B)");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        log("[D] done");
    });

    jobs.add_dependency(c, a);
    jobs.add_dependency(d, a);
    jobs.add_dependency(d, b);

    std::cout << "Running job batch...\n\n";

    jobs.run();
    jobs.wait();

    std::cout << "\nAll jobs completed.\n";
}