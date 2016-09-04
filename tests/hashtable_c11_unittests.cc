#include <sparsehash/dense_hash_map>
#include <unordered_map>

#include "hashtable_test_interface.h"
#include "gtest/gtest.h"

using google::dense_hash_map;
using google::dense_hash_set;

namespace sparsehash_internal = google::sparsehash_internal;

TEST(HashtableMoveTest, Insert_RValue)
{
    dense_hash_map<int, std::unique_ptr<int>> h;
    h.set_empty_key(0);

    auto p1 = std::make_pair(5, std::unique_ptr<int>(new int(1234)));
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

struct A
{
    A() =default;

    A(const A&) =delete;
    A& operator=(const A&) =delete;

    A(A&& a) { move_ctor = a.move_ctor + 1; }
    A& operator=(A&& a) { move_assign = a.move_assign + 1; return *this; }

    int move_ctor = 0;
    int move_assign = 0;
};

TEST(HashtableMoveTest, MoveCount)
{
    dense_hash_map<int, A> h(10);
    h.set_empty_key(0);

    h.insert(std::make_pair(5, A()));
    ASSERT_EQ(0, h[5].move_assign);
    ASSERT_EQ(3, h[5].move_ctor);
}
