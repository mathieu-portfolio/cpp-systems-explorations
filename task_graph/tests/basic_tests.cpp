#include <gtest/gtest.h>

#include "task_graph.hpp"

#include <atomic>

TEST(TaskGraphBasic, CanCreateTasks)
{
    TaskGraph graph(2);

    const TaskId a = graph.add_task([] {});
    const TaskId b = graph.add_named_task("named", [] {});

    EXPECT_NE(a.value, b.value);
}

TEST(TaskGraphBasic, ExecutesIndependentTasks)
{
    std::atomic<int> counter{0};

    TaskGraph graph(2);

    graph.add_task([&] {
        counter.fetch_add(1, std::memory_order_relaxed);
    });

    graph.add_task([&] {
        counter.fetch_add(1, std::memory_order_relaxed);
    });

    graph.run();
    graph.wait();

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 2);
}

TEST(TaskGraphBasic, EmptyGraphReturnsImmediately)
{
    TaskGraph graph(2);

    graph.run();
    graph.wait();

    SUCCEED();
}
