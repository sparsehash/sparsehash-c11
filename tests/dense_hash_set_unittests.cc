//
// Created by Lukas Barth on 17.04.18.
//

#include "gtest/gtest.h"
#include "sparsehash/dense_hash_set"

using google::dense_hash_set;

TEST(DenseHashSet, TestEmplaceHint) {
	dense_hash_set<const char *> set;
	set.set_empty_key(nullptr);

	const char * str1 = "Hello";
	const char * str2 = "World";

	set.insert(str1);
	auto it = set.begin();
	set.emplace_hint(it, str2);

	ASSERT_EQ(set.size(), 2ul);
}

TEST(DenseHashSet, TestEmplaceHintAfterDelete) {
	dense_hash_set<const char *> set;

	const char * deleted_ptr = "";
	const char * str1 = "Hello";

	set.set_empty_key(nullptr);
	set.set_deleted_key(deleted_ptr);

	auto insertion_result = set.insert(str1);
	auto str1_inserted_it = insertion_result.first;

	auto deleted_iterator = set.erase(str1_inserted_it);
	set.emplace_hint(deleted_iterator, str1);
}