#include <gtest/gtest.h>

#include "task_graph.hpp"

#include <atomic>
#include <stdexcept>

TEST(TaskGraphPipeline, CanAddEdgeBeforeRun)
{
    TaskGraph graph(2);

    const TaskId a = graph.add_task([] {});
    const TaskId b = graph.add_task([] {});

    EXPECT_NO_THROW(graph.add_edge(a, b));
}

TEST(TaskGraphPipeline, RejectsOutOfRangeSourceTaskId)
{
    TaskGraph graph(2);

    const TaskId a = graph.add_task([] {});

    EXPECT_THROW(graph.add_edge(TaskId{999}, a), std::out_of_range);
}

TEST(TaskGraphPipeline, RejectsOutOfRangeTargetTaskId)
{
    TaskGraph graph(2);

    const TaskId a = graph.add_task([] {});

    EXPECT_THROW(graph.add_edge(a, TaskId{999}), std::out_of_range);
}

TEST(TaskGraphPipeline, RejectsSelfEdge)
{
    TaskGraph graph(2);

    const TaskId a = graph.add_task([] {});

    EXPECT_THROW(graph.add_edge(a, a), std::invalid_argument);
}

TEST(TaskGraphPipeline, RejectsAddingEdgeAfterRunStarts)
{
    TaskGraph graph(2);

    const TaskId a = graph.add_task([] {});
    const TaskId b = graph.add_task([] {});

    graph.run();
    EXPECT_THROW(graph.add_edge(a, b), std::logic_error);
}

TEST(TaskGraphPipeline, ExecutesDependentTasksInOrder)
{
    std::atomic<int> state{0};

    TaskGraph graph(2);

    const TaskId a = graph.add_task([&] {
        state.store(1, std::memory_order_release);
    });

    const TaskId b = graph.add_task([&] {
        EXPECT_EQ(state.load(std::memory_order_acquire), 1);
        state.store(2, std::memory_order_release);
    });

    graph.add_edge(a, b);

    graph.run();
    graph.wait();

    EXPECT_EQ(state.load(std::memory_order_acquire), 2);
}

TEST(TaskGraphPipeline, FanInTaskRunsAfterAllInputs)
{
    std::atomic<int> completed_inputs{0};
    std::atomic<int> observed{0};

    TaskGraph graph(3);

    const TaskId a = graph.add_task([&] {
        completed_inputs.fetch_add(1, std::memory_order_acq_rel);
    });

    const TaskId b = graph.add_task([&] {
        completed_inputs.fetch_add(1, std::memory_order_acq_rel);
    });

    const TaskId c = graph.add_task([&] {
        observed.store(completed_inputs.load(std::memory_order_acquire), std::memory_order_release);
    });

    graph.add_edge(a, c);
    graph.add_edge(b, c);

    graph.run();
    graph.wait();

    EXPECT_EQ(observed.load(std::memory_order_acquire), 2);
}
