cmake_minimum_required(VERSION 3.16)

project(ece309_project2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Main program
add_executable(miniharness
    src/main.cpp
    src/conversation.cpp
    src/sentinel_scanner.cpp
    src/model_client.cpp
    src/scripted_client.cpp
    src/replay_client.cpp
    src/harness.cpp
)

target_compile_options(miniharness PRIVATE -fsanitize=address)
target_link_options(miniharness PRIVATE -fsanitize=address)


# Project 2 tests
add_executable(test_p2
    tests/p2/test_p2.cpp
    src/conversation.cpp
    src/sentinel_scanner.cpp
    src/model_client.cpp
    src/scripted_client.cpp
    src/replay_client.cpp
    src/harness.cpp
)

target_compile_options(test_p2 PRIVATE -fsanitize=address)
target_link_options(test_p2 PRIVATE -fsanitize=address)