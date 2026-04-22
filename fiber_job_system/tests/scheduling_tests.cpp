#include <gtest/gtest.h>

#include "fiber_job_system.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

namespace
{
class CountingTask final : public FiberTask
{
public:
    CountingTask(std::atomic<int>& phase, std::atomic<int>& completed)
        : phase_(phase), completed_(completed)
    {
    }

    FiberStepResult run() override
    {
        const int current = phase_.load(std::memory_order_relaxed);
        if (current == 0)
        {
            phase_.store(1, std::memory_order_relaxed);
            return FiberStepResult::Yield;
        }

        completed_.fetch_add(1, std::memory_order_relaxed);
        return FiberStepResult::Complete;
    }

private:
    std::atomic<int>& phase_;
    std::atomic<int>& completed_;
};
}

TEST(FiberJobSystemScheduling, ExecutesSubmittedWork)
{
    std::atomic<int> counter{0};

    {
        FiberJobSystem scheduler(2);
        scheduler.submit([&] {
            counter.fetch_add(1, std::memory_order_relaxed);
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 1);
}

TEST(FiberJobSystemScheduling, OneShotYieldSuspendsAndResumeRequeuesFiber)
{
    std::atomic<int> runs{0};

    {
        FiberJobSystem scheduler(2);

        scheduler.submit([&] {
            const int current = runs.fetch_add(1, std::memory_order_relaxed);
            if (current == 0)
            {
                scheduler.yield_current();
            }
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        EXPECT_EQ(runs.load(std::memory_order_relaxed), 1);

        scheduler.resume_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    EXPECT_EQ(runs.load(std::memory_order_relaxed), 2);
}

TEST(FiberJobSystemScheduling, TaskBasedFiberContinuesLogicalProgressAfterResume)
{
    std::atomic<int> phase{0};
    std::atomic<int> completed{0};

    {
        FiberJobSystem scheduler(2);

        scheduler.submit_task(std::make_unique<CountingTask>(phase, completed));

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        EXPECT_EQ(phase.load(std::memory_order_relaxed), 1);
        EXPECT_EQ(completed.load(std::memory_order_relaxed), 0);

        scheduler.resume_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    EXPECT_EQ(completed.load(std::memory_order_relaxed), 1);
}

TEST(FiberJobSystemScheduling, OtherWorkCanRunWhileFiberIsSuspended)
{
    std::atomic<int> counter{0};

    {
        FiberJobSystem scheduler(2);

        scheduler.submit([&] {
            scheduler.yield_current();
        });

        scheduler.submit([&] {
            counter.fetch_add(1, std::memory_order_relaxed);
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 1);
}
