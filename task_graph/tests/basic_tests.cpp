#include <gtest/gtest.h>

#include "task_graph.hpp"

TEST(TaskGraphBasic, CanCreateTasks)
{
    TaskGraph graph(2);

    const TaskId a = graph.add_task([] {});
    const TaskId b = graph.add_named_task("named", [] {});

    EXPECT_NE(a.value, b.value);
}
