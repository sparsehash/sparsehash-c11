//
// Created by Lukas Barth on 17.04.18.
//

#include "gtest/gtest.h"
#include "sparsehash/dense_hash_map"

using google::dense_hash_map;

TEST(DenseHashMap, TestEmplaceHint) {
	dense_hash_map<int, const char *> map;
	map.set_empty_key(0);

	const char * str1 = "Hello";

	map.insert({42, str1});
	auto it = map.begin();
	map.emplace_hint(it, 1701, "World");

	ASSERT_EQ(map.size(), 2);
}