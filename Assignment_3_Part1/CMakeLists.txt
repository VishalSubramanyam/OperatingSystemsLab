cmake_minimum_required(VERSION 3.24)
project(assignment3)

set(CMAKE_CXX_STANDARD 17)

add_executable(assignment3 main.cpp graph.cpp)
add_executable(test test.cpp graph.cpp)
