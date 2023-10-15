cmake_minimum_required(VERSION 3.25)
project(assignment6)

set(CMAKE_CXX_STANDARD 17)


add_library(goodMalloc STATIC goodMalloc/goodmalloc.cpp goodMalloc/goodmalloc.h)
target_include_directories(goodMalloc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(assignment6 mergesort.cpp)
target_link_libraries(assignment6 goodMalloc)
target_include_directories(assignment6 PUBLIC goodMalloc)