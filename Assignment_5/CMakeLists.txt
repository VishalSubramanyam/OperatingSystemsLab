cmake_minimum_required(VERSION 3.24)
project(assignment5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(assignment5 main.cpp)
target_link_libraries(assignment5 pthread)
