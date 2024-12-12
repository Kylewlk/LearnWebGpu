cmake_minimum_required(VERSION 3.20)

set(target_name LearnWebGPU)
project(${target_name})


set(WEB_GPU_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/dawn/Release)
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libs/dawn/Debug/bin/webgpu_dawn.dll)
    set(WEB_GPU_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/dawn/Debug)
    message(STATUS "Use Debug dawn: ${WEB_GPU_DIR}")
endif ()
find_package(Dawn REQUIRED CONFIG HINTS ${WEB_GPU_DIR})

set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/Release)
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/Debug/lib/glfw3.lib)
    set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/Debug)
    message(STATUS "Use Debug glfw: ${GLFW_DIR}")
endif ()
find_package(glfw3 REQUIRED CONFIG HINTS ${GLFW_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/run)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/run)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/run)

file(GLOB WEB_GPU_BIN "${WEB_GPU_DIR}/bin/*")
#message(STATUS ${WEB_GPU_BIN})
foreach (bin_file ${WEB_GPU_BIN})
    file(COPY "${bin_file}" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endforeach ()

set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/common)
file(GLOB_RECURSE common_src LIST_DIRECTORIES false
        ${COMMON_DIR}/*.h
        ${COMMON_DIR}/*.hpp
        ${COMMON_DIR}/*.c
        ${COMMON_DIR}/*.cpp
)
add_library(common_lib INTERFACE)
target_include_directories(common_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_sources(common_lib INTERFACE ${common_src})
target_link_libraries(common_lib INTERFACE dawn::webgpu_dawn glfw)
target_compile_options(common_lib INTERFACE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
target_compile_definitions(common_lib INTERFACE CURRENT_WORKING_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(src/01-hello-web-gpu)
add_subdirectory(src/02-triangle)