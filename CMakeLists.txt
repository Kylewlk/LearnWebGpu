cmake_minimum_required(VERSION 3.20)

set(target_name LearnWebGPU)
project(${target_name})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

#message(STATUS ${CMAKE_SYSTEM_NAME})
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "MAC OS")
    set(SYSTEM_DIR Mac)
    set(MAC TRUE)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Windows")
    set(SYSTEM_DIR Win)
    set(WIN TRUE)
endif ()

set(THIRD_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)


set(WEB_GPU_DIR ${THIRD_LIB_DIR}/dawn/${SYSTEM_DIR}/Release)
if (EXISTS ${THIRD_LIB_DIR}/dawn/${SYSTEM_DIR}/Debug/bin/webgpu_dawn.dll)
    set(WEB_GPU_DIR ${THIRD_LIB_DIR}/dawn/${SYSTEM_DIR}/Debug)
    message(STATUS "Use Debug dawn: ${WEB_GPU_DIR}")
endif ()
find_package(Dawn REQUIRED CONFIG HINTS ${WEB_GPU_DIR})

set(GLFW_DIR ${THIRD_LIB_DIR}/glfw/${SYSTEM_DIR}/Release)
if (EXISTS ${THIRD_LIB_DIR}/glfw/${SYSTEM_DIR}/Debug/lib/glfw3.lib)
    set(GLFW_DIR ${THIRD_LIB_DIR}/glfw/${SYSTEM_DIR}/Debug)
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
if (MAC)
    list(APPEND common_src ${COMMON_DIR}/Utils.mm)
endif ()
add_library(common_lib ${common_src})
target_include_directories(common_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${THIRD_LIB_DIR}/include)
target_link_libraries(common_lib PUBLIC dawn::webgpu_dawn glfw)
if (MAC)
   target_link_libraries(common_lib PUBLIC "-framework Metal" "-framework QuartzCore")
endif ()

target_compile_options(common_lib PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/utf-8>> )
target_compile_definitions(common_lib PUBLIC CURRENT_WORKING_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

function(add_example name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} common_lib)
    target_compile_definitions(${name} PUBLIC EXAMPLE_NAME="${name}")
endfunction()

add_subdirectory(src/basic basic)