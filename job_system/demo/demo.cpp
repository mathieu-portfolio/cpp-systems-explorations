#include "job_system.hpp"

#include <iostream>
#include <mutex>

int main()
{
    std::mutex cout_mutex;

    JobSystem jobs(4);

    const JobId a = jobs.create_job([&] {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "job A: prepare data\n";
    });

    const JobId b = jobs.create_job([&] {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "job B: independent work\n";
    });

    const JobId c = jobs.create_job([&] {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "job C: depends on A\n";
    });

    const JobId d = jobs.create_job([&] {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "job D: depends on A and B\n";
    });

    jobs.add_dependency(c, a);
    jobs.add_dependency(d, a);
    jobs.add_dependency(d, b);

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Running job batch...\n";
    }

    jobs.run();
    jobs.wait();

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "All jobs completed.\n";
    }

    return 0;
}