#include "job_system.hpp"

#include <iostream>

int main()
{
    JobSystem jobs(4);

    const JobId a = jobs.create_job([] {
        std::cout << "job A\n";
    });

    const JobId b = jobs.create_job([] {
        std::cout << "job B\n";
    });

    (void)a;
    (void)b;

    std::cout << "Job system skeleton created.\n";
    std::cout << "Dependencies, run(), and wait() are not implemented yet.\n";

    return 0;
}
