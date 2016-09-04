#include <sparsehash/dense_hash_map>
#include <unordered_map>

#include "hashtable_test_interface.h"
#include "gtest/gtest.h"

using google::dense_hash_map;
using google::dense_hash_set;

namespace sparsehash_internal = google::sparsehash_internal;

using namespace testing;

template <typename K, typename V>
//using ht = std::unordered_map<K, V>;
using ht = dense_hash_map<K, V>;

TEST(HashtableTest, Move)
{
    using hti = ht<int, std::unique_ptr<int>>;

    hti h;
    typename hti::value_type p;
    h.insert(std::move(p));
}
