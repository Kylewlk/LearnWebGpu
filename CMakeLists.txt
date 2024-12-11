cmake_minimum_required(VERSION 3.20)

set(target_name LearnWebGPU)
project(${target_name})


find_package(Dawn REQUIRED CONFIG HINTS libs/dawn/Release)

add_subdirectory(src/01-hello-web-gpu)