#include "work_stealing_pool.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

namespace
{
    constexpr const char* kColors[] = {
        "\033[90m", // gray
        "\033[31m", // red
        "\033[32m", // green
        "\033[33m", // yellow
        "\033[34m", // blue
        "\033[35m", // magenta
        "\033[36m", // cyan
        "\033[37m", // white

        "\033[37m", // white
        "\033[91m", // bright red
        "\033[92m", // bright green
        "\033[93m", // bright yellow
        "\033[94m", // bright blue
        "\033[95m", // bright magenta
        "\033[96m", // bright cyan
        "\033[97m"  // bright white
    };

    constexpr const char* kReset = "\033[0m";

    const char* job_color(int job_id)
    {
        return kColors[job_id % 16];
    }
}

int main()
{
    std::mutex cout_mutex;

    auto log = [&](int job_id, bool slow, const char* phase)
    {
        const char* color = job_color(job_id);

        std::lock_guard<std::mutex> lock(cout_mutex);

        std::cout
            << color
            << "[job " << job_id << "] "
            << (slow ? "[SLOW] " : "[FAST] ")
            << phase
            << " (thread " << std::this_thread::get_id() << ")"
            << kReset
            << '\n';
    };

    std::cout << "Work-stealing pool demo (job-colored)\n\n";

    std::cout << "Legend:\n";
    std::cout << "  Each job has a unique color\n";
    std::cout << "  [SLOW] = long job\n";
    std::cout << "  [FAST] = short job\n\n";

    std::cout << "What to observe:\n";
    std::cout << "  - Each job keeps the same color from start to end\n";
    std::cout << "  - Logs interleave → multiple jobs run in parallel\n";
    std::cout << "  - Slow jobs overlap with many fast jobs\n\n";

    {
        WorkStealingPool pool(4);

        for (int i = 0; i < 16; ++i)
        {
            const bool slow = (i % 4 == 0);

            pool.submit([&, i, slow]
            {
                log(i, slow, "start");

                if (slow)
                    std::this_thread::sleep_for(std::chrono::milliseconds(150));
                else
                    std::this_thread::sleep_for(std::chrono::milliseconds(30));

                log(i, slow, "done");
            });
        }

        std::cout << "Submitted 16 jobs (4 slow, 12 fast)\n\n";
    }

    std::cout << "\nAll jobs completed.\n";
    return 0;
}