#include <gtest/gtest.h>
#include "task_graph.hpp"

TEST(TaskGraphNames, RejectDuplicateNames) {
    TaskGraph g(2);
    g.add_named_task("A", []{});
    EXPECT_THROW(g.add_named_task("A", []{}), std::logic_error);
}

TEST(TaskGraphNames, LookupWorks) {
    TaskGraph g(2);
    g.add_named_task("A", []{});
    g.add_named_task("B", []{});
    EXPECT_NO_THROW(g.add_edge("A","B"));
}

TEST(TaskGraphNames, UnknownNameFails) {
    TaskGraph g(2);
    g.add_named_task("A", []{});
    EXPECT_THROW(g.add_edge("A","B"), std::out_of_range);
}
