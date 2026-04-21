#include <gtest/gtest.h>

#include "task_graph.hpp"

TEST(TaskGraphPipeline, AddEdgeIsNotImplementedYet)
{
    TaskGraph graph(2);

    const TaskId a = graph.add_task([] {});
    const TaskId b = graph.add_task([] {});

    EXPECT_THROW(graph.add_edge(a, b), std::logic_error);
}
