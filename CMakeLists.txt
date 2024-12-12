cmake_minimum_required(VERSION 3.20)

set(target_name LearnWebGPU)
project(${target_name})


set(WEB_GPU_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/dawn/Release)
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libs/dawn/Debug/bin/webgpu_dawn.dll)
    set(WEB_GPU_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/dawn/Debug)
    message(STATUS "Use Debug dawn: ${WEB_GPU_DIR}")
endif ()

find_package(Dawn REQUIRED CONFIG HINTS ${WEB_GPU_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/run)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/run)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/run)

file(GLOB WEB_GPU_BIN "${WEB_GPU_DIR}/bin/*")
#message(STATUS ${WEB_GPU_BIN})
foreach (bin_file ${WEB_GPU_BIN})
    file(COPY "${bin_file}" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endforeach ()

add_subdirectory(src/01-hello-web-gpu)