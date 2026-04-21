#include <gtest/gtest.h>

#include "task_graph.hpp"

#include <atomic>
#include <stdexcept>

TEST(TaskGraphExceptions, ThrowingTaskDoesNotBlockGraphCompletion)
{
    std::atomic<int> counter{0};

    TaskGraph graph(2);

    graph.add_task([] {
        throw std::runtime_error("task failure");
    });

    graph.add_task([&] {
        counter.fetch_add(1, std::memory_order_relaxed);
    });

    graph.run();
    graph.wait();

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 1);
}

TEST(TaskGraphExceptions, DependentsStillRunAfterThrowingPrerequisite)
{
    std::atomic<int> state{0};

    TaskGraph graph(2);

    const TaskId load = graph.add_task([&] {
        state.store(1, std::memory_order_release);
        throw std::runtime_error("task failure");
    });

    const TaskId parse = graph.add_task([&] {
        EXPECT_EQ(state.load(std::memory_order_acquire), 1);
        state.store(2, std::memory_order_release);
    });

    graph.add_edge(load, parse);

    graph.run();
    graph.wait();

    EXPECT_EQ(state.load(std::memory_order_acquire), 2);
}
