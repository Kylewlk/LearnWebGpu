cmake_minimum_required(VERSION 3.20)


project(WebGPU-example)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

add_example(01-HelloWebGpu)
add_example(02-Triangle)
add_example(03-TriangleMSAA)