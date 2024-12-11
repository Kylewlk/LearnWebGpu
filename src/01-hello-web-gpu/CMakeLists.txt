cmake_minimum_required(VERSION 3.20)

set(target_name HelloWebGPU)
project(${target_name})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)


add_executable(${target_name} HelloWebGpu.cpp)

target_link_libraries(${target_name} dawn::webgpu_dawn)

target_compile_options(${target_name} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
target_compile_definitions(${target_name} PRIVATE CURRENT_WORKING_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
