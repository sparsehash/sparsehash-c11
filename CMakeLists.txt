cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Release)
project (sparsehash)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wpedantic -Wno-missing-field-initializers")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

include_directories(.)

enable_testing()
add_subdirectory(tests)

install(DIRECTORY sparsehash DESTINATION include/google)

