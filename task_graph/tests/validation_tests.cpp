#include <gtest/gtest.h>
#include "task_graph.hpp"

TEST(TaskGraphValidation, RejectsCycle) {
    TaskGraph g(2);
    auto a=g.add_task([]{});
    auto b=g.add_task([]{});
    g.add_edge(a,b);
    g.add_edge(b,a);
    EXPECT_THROW(g.run(), std::logic_error);
}

TEST(TaskGraphValidation, RejectsDuplicateEdge) {
    TaskGraph g(2);
    auto a=g.add_task([]{});
    auto b=g.add_task([]{});
    g.add_edge(a,b);
    EXPECT_THROW(g.add_edge(a,b), std::logic_error);
}
