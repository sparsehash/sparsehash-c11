//
// Created by Lukas Barth on 17.04.18.
//

#include "gtest/gtest.h"
#include "sparsehash/dense_hash_set"

using google::dense_hash_set;

TEST(DenseHashSet, TestEmplaceHint) {
	dense_hash_set<const char *> set;

	const char * str1 = "Hello";
	const char * str2 = "World";

	set.insert(str1);
	auto it = set.begin();
	set.emplace_hint(it, str2);

	ASSERT_EQ(set.size(), 2);
}