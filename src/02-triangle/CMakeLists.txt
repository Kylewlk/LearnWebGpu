cmake_minimum_required(VERSION 3.20)

set(target_name Triangle)
project(${target_name})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)


add_executable(${target_name} Triangle.cpp)

target_link_libraries(${target_name} common_lib)

