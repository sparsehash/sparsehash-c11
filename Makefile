TEST_DIR = tests

CPPFLAGS += -I$(TEST_DIR) -I. -isystem $(TEST_DIR)/gtest
CXXFLAGS += -Wall -Wextra -Wpedantic -Wno-missing-field-initializers -std=c++11 -g -O3
LDFLAGS += -lpthread

all : sparsehash_unittests 

check : all
	./sparsehash_unittests 

clean :
	rm -rf sparsehash_unittests *.o

gmock-gtest-all.o : 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/gtest/gmock-gtest-all.cc

sparsetable_unittests.o : $(TEST_DIR)/sparsetable_unittests.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/sparsetable_unittests.cc 

testmain.o : $(TEST_DIR)/sparsetable_unittests.cc 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/testmain.cc 	

sparsehash_unittests : sparsetable_unittests.o testmain.o gmock-gtest-all.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
