cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Release)
project (sparsehash)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(.)

enable_testing()
add_subdirectory(tests)

