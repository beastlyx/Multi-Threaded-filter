cmake_minimum_required(VERSION 3.28)
project(module_6 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
add_executable(module_6 BmpProcessor.c BaseFilters.c)
target_link_libraries(module_6 m)