#include <sparsehash/dense_hash_map>
#include <unordered_map>

#include "hashtable_test_interface.h"
#include "gtest/gtest.h"

using google::dense_hash_map;
using google::dense_hash_set;

namespace sparsehash_internal = google::sparsehash_internal;

struct HashtableMoveTest : public ::testing::Test
{
    HashtableMoveTest() { h.set_empty_key(0); }

    template <typename K, typename V>
    using hashtable = dense_hash_map<K, V>;
    //using hashtable = std::unordered_map<K, V>;

    using ht = hashtable<int, std::unique_ptr<int>>;
    ht h;
};

TEST_F(HashtableMoveTest, Insert_RValue)
{
    typename ht::value_type p1 = std::make_pair(5, std::unique_ptr<int>(new int(1234)));

    auto p = h.insert(std::move(p1));
    ASSERT_EQ(true, p.second);
    ASSERT_EQ(5, p.first->first);
    ASSERT_EQ(1234, *p.first->second);

    p = h.insert(std::make_pair(10, std::unique_ptr<int>(new int(5678))));
    ASSERT_EQ(true, p.second);
    ASSERT_EQ(10, p.first->first);
    ASSERT_EQ(5678, *p.first->second);

    ASSERT_EQ(2, (int)h.size());
}
