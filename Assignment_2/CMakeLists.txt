cmake_minimum_required(VERSION 3.10)

project(assignment2 LANGUAGES C CXX)

add_executable(shell Assignment2_7_20CS10081_20CS10075_20CS10038_20CS10019.cpp)
target_link_libraries(shell readline)
