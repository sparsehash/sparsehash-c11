include_directories(.)

add_subdirectory(gtest)

add_executable(sparsehash_unittests
    testmain.cc
    sparsetable_unittests.cc
    simple_unittests.cc
    hashtable_unittests.cc
    hashtable_c11_unittests.cc
    fixture_unittests.cc
    allocator_unittests.cc
    dense_hash_set_unittests.cc
    dense_hash_map_unittests.cc)

add_executable(bench bench.cc)

add_test(sparsehash_unittests sparsehash_unittests)
target_link_libraries(sparsehash_unittests gtest pthread)
target_link_libraries(bench pthread)
