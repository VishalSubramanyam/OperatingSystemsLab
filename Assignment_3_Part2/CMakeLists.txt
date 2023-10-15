cmake_minimum_required(VERSION 3.24)
project(assignment3)

set(CMAKE_CXX_STANDARD 17)

add_executable(assignment3 main.cpp graph.cpp)
target_link_libraries(assignment3 rt)

add_executable(main main.cpp graph.cpp)
add_executable(producer producer.cpp graph.cpp)
add_executable(consumer consumer.cpp graph.cpp)
add_executable(test test.cpp graph.cpp)
add_executable(print_nbrs print_nbrs.cpp graph.cpp)