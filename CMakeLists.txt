cmake_minimum_required(VERSION 3.10)
project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++17 -pedantic -pthread")

include_directories("src/include")

file(
    GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(result.o ${SOURCES})
