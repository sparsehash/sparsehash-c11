TEST_DIR = tests

CPPFLAGS += -I$(TEST_DIR) -I. -isystem $(TEST_DIR)/gtest
CXXFLAGS += -Wall -Wextra -Wpedantic -Wno-missing-field-initializers -std=c++11 -O3
LDFLAGS += -lpthread

DENSE_HEADERS = sparsehash/dense_hash_set sparsehash/dense_hash_map sparsehash/internal/densehashtable.h sparsehash/internal/hashtable-common.h
TEST_HEADERS = $(TEST_DIR)/fixture_unittests.h $(TEST_DIR)/hashtable_test_interface.h
HEADERS = $(DENSE_HEADERS) $(TEST_HEADER)

all : sparsehash_unittests bench

check : all
	./sparsehash_unittests 

clean :
	rm -rf sparsehash_unittests *.o

bench.o : $(TEST_DIR)/bench.cc $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/bench.cc 

bench: bench.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

gmock-gtest-all.o : 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/gtest/gmock-gtest-all.cc

fixture_unittests.o : $(TEST_DIR)/fixture_unittests.cc $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/fixture_unittests.cc

simple_unittests.o : $(TEST_DIR)/simple_unittests.cc $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/simple_unittests.cc 

sparsetable_unittests.o : $(TEST_DIR)/sparsetable_unittests.cc $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/sparsetable_unittests.cc 

allocator_unittests.o : $(TEST_DIR)/allocator_unittests.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/allocator_unittests.cc 

hashtable_unittests.o: $(TEST_DIR)/hashtable_unittests.cc $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/hashtable_unittests.cc 

hashtable_c11_unittests.o: $(TEST_DIR)/hashtable_c11_unittests.cc $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/hashtable_c11_unittests.cc 

testmain.o : $(TEST_DIR)/*.cc 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/testmain.cc 	

sparsehash_unittests : simple_unittests.o sparsetable_unittests.o allocator_unittests.o hashtable_unittests.o hashtable_c11_unittests.o fixture_unittests.o testmain.o gmock-gtest-all.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
