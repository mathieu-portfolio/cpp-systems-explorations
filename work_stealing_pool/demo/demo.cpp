#include "work_stealing_pool.hpp"

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

    {
        WorkStealingPool pool(4);

        for (int i = 0; i < 8; ++i)
        {
            pool.submit([&, i] {
                log("[job] start");
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
                log("[job] done");
            });
        }

        log("Submitted 8 jobs to the baseline pool.");
    }

    std::cout << "All jobs completed before destruction.\n";
    return 0;
}
