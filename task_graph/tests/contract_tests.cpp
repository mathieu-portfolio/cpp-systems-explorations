#include <gtest/gtest.h>

#include "task_graph.hpp"

#include <functional>
#include <stdexcept>

TEST(TaskGraphContract, RejectsEmptyTask)
{
    TaskGraph graph(2);

    EXPECT_THROW(
        {
            graph.add_task(std::function<void()>{});
        },
        std::invalid_argument);
}

TEST(TaskGraphContract, RejectsEmptyNamedTask)
{
    TaskGraph graph(2);

    EXPECT_THROW(
        {
            graph.add_named_task("x", std::function<void()>{});
        },
        std::invalid_argument);
}

TEST(TaskGraphContract, RejectsCreatingTasksAfterRunStarts)
{
    TaskGraph graph(2);

    graph.run();

    EXPECT_THROW(
        {
            graph.add_task([] {});
        },
        std::logic_error);
}

TEST(TaskGraphContract, RejectsRunMoreThanOnce)
{
    TaskGraph graph(2);

    graph.run();
    EXPECT_THROW(graph.run(), std::logic_error);
}
