cmake_minimum_required(VERSION 3.3)

set(CMAKE_BUILD_TYPE Release)
project (sparsehash
         VERSION 2.11.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wpedantic -Wno-missing-field-initializers")
endif()
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

include_directories(.)

enable_testing()
add_subdirectory(tests)

install(DIRECTORY sparsehash DESTINATION include/google)

