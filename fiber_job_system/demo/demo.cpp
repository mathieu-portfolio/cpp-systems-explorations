#include "fiber_job_system.hpp"

#include <iostream>

int main()
{
    FiberJobSystem scheduler(4);

    std::cout << "Fiber job system skeleton created.\n";
    std::cout << "Execution model now includes explicit cooperative yield semantics.\n";
    std::cout << "submit() and yield_current() are not implemented yet.\n";

    return 0;
}
