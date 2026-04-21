#include "task_graph.hpp"

#include <iostream>
#include <mutex>

int main()
{
    std::mutex cout_mutex;

    auto log = [&](const char* msg) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << msg << '\n';
    };

    std::cout << "Pipeline graph:\n";
    std::cout << "load -> parse -> transform -> save\n";
    std::cout << "load -> validate\n\n";

    TaskGraph graph(4);

    const TaskId load = graph.add_named_task("load", [&] {
        log("[load] done");
    });

    const TaskId parse = graph.add_named_task("parse", [&] {
        log("[parse] done");
    });

    const TaskId transform = graph.add_named_task("transform", [&] {
        log("[transform] done");
    });

    const TaskId save = graph.add_named_task("save", [&] {
        log("[save] done");
    });

    const TaskId validate = graph.add_named_task("validate", [&] {
        log("[validate] done");
    });

    graph.add_edge(load, parse);
    graph.add_edge(parse, transform);
    graph.add_edge(transform, save);
    graph.add_edge(load, validate);

    std::cout << "Running graph...\n\n";

    graph.run();
    graph.wait();

    std::cout << "\nAll tasks completed.\n";
    return 0;
}
