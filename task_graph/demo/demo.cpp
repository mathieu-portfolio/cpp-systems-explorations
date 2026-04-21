#include "task_graph.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

int main()
{
    std::mutex cout_mutex;

    auto log = [&](const char* msg) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << msg << '\n';
    };

    std::cout << "Task graph:\n";
    std::cout << "  load -> parse -> transform ----\\\n";
    std::cout << "    \\                            -> package -> publish\n";
    std::cout << "     -> validate ---------------/\n\n";

    std::cout << "Expected behavior:\n";
    std::cout << "  - load runs first\n";
    std::cout << "  - parse and validate unlock after load\n";
    std::cout << "  - transform unlocks after parse\n";
    std::cout << "  - package waits for both transform and validate\n";
    std::cout << "  - publish runs last\n\n";

    TaskGraph graph(4);

    const TaskId load = graph.add_named_task("load", [&] {
        log("[load] start");
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        log("[load] done");
    });

    const TaskId parse = graph.add_named_task("parse", [&] {
        log("[parse] start");
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        log("[parse] done");
    });

    const TaskId transform = graph.add_named_task("transform", [&] {
        log("[transform] start");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        log("[transform] done");
    });

    const TaskId validate = graph.add_named_task("validate", [&] {
        log("[validate] start");
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        log("[validate] done");
    });

    const TaskId package = graph.add_named_task("package", [&] {
        log("[package] start");
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        log("[package] done");
    });

    const TaskId publish = graph.add_named_task("publish", [&] {
        log("[publish] start");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        log("[publish] done");
    });

    graph.add_edge(load, parse);
    graph.add_edge(load, validate);
    graph.add_edge(parse, transform);
    graph.add_edge(transform, package);
    graph.add_edge(validate, package);
    graph.add_edge(package, publish);

    std::cout << "Running graph...\n\n";

    graph.run();
    graph.wait();

    std::cout << "\nAll tasks completed.\n";
    return 0;
}