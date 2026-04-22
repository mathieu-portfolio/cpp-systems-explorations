#include "fiber_job_system.hpp"
#include <iostream>
#include <thread>

int main()
{
    FiberJobSystem s(2);

    int step = 0;

    s.submit_resumable([&]{
        if (step == 0)
        {
            std::cout << "step 1\n";
            step++;
            return FiberStepResult::Yield;
        }
        std::cout << "step 2\n";
        return FiberStepResult::Complete;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "resume\n";
    s.resume_all();
}
