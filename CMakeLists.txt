cmake_minimum_required(VERSION 3.14)
project(main LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

include_directories("src/include")

file(
    GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.cu
)

add_executable(result.o ${SOURCES})
