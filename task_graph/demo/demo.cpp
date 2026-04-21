#include "task_graph.hpp"

#include <iostream>

int main()
{
    TaskGraph graph(4);

    const TaskId load = graph.add_named_task("load", [] {
        std::cout << "load\n";
    });

    const TaskId parse = graph.add_named_task("parse", [] {
        std::cout << "parse\n";
    });

    (void)load;
    (void)parse;

    std::cout << "Task graph skeleton created.\n";
    std::cout << "Edges, run(), and wait() are not implemented yet.\n";

    return 0;
}
