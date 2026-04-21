cmake_minimum_required(VERSION 3.16)

project(task_graph LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------
# JobSystem dependency
# -----------------------------
add_subdirectory(../job_system job_system_build)

# -----------------------------
# Library
# -----------------------------
add_library(task_graph
    src/task_graph.cpp
)

target_link_libraries(task_graph
    PUBLIC job_system
)

target_include_directories(task_graph
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# -----------------------------
# Demo
# -----------------------------
add_executable(task_graph_demo
    demo/demo.cpp
)

target_link_libraries(task_graph_demo
    PRIVATE task_graph
)

# -----------------------------
# Testing
# -----------------------------
include(CTest)

if(BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
    )

    FetchContent_MakeAvailable(googletest)

    add_executable(task_graph_tests
        tests/basic_tests.cpp
        tests/pipeline_tests.cpp
        tests/contract_tests.cpp
        tests/validation_tests.cpp
        tests/name_tests.cpp
        tests/exception_tests.cpp
    )

    target_link_libraries(task_graph_tests
        PRIVATE
            task_graph
            GTest::gtest_main
    )

    target_include_directories(task_graph_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    include(GoogleTest)
    gtest_discover_tests(task_graph_tests)
endif()
