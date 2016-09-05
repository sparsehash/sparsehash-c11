#include <sparsehash/dense_hash_map>
#include <unordered_map>

#include "hashtable_test_interface.h"
#include "gtest/gtest.h"
#include <unordered_map>
#include <unordered_set>
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

TEST(HashtableMoveTest, Emplace)
{
    dense_hash_map<int, std::unique_ptr<int>> h;
    h.set_empty_key(0);

    auto p = h.emplace(5, new int(1234));
    ASSERT_EQ(true, p.second);
    ASSERT_EQ(5, p.first->first);
    ASSERT_EQ(1234, *p.first->second);

    p = h.emplace(10, new int(5678));
    ASSERT_EQ(true, p.second);
    ASSERT_EQ(10, p.first->first);
    ASSERT_EQ(5678, *p.first->second);

    ASSERT_EQ(2, (int)h.size());
}

struct A
{
    A() =default;
    A(int i) noexcept : _i(i) {}

    A(const A&) =delete;
    A& operator=(const A&) =delete;

    A(A&& a) { move_ctor = a.move_ctor + 1; }
    A& operator=(A&& a) { move_assign = a.move_assign + 1; return *this; }

    int _i = 0;
    int move_ctor = 0;
    int move_assign = 0;
};

bool operator==(const A& a1, const A& a2) { return a1._i < a2._i; }

struct HashA
{
    std::size_t operator()(const A& a) const { return std::hash<int>()(a._i); }
};

TEST(HashtableMoveTest, MoveCount)
{
    dense_hash_map<int, A> h(10);
    h.set_empty_key(0);

    h.insert(std::make_pair(5, A()));
    ASSERT_EQ(0, h[5].move_assign);
    ASSERT_EQ(3, h[5].move_ctor);
}

TEST(HashtableMoveTest, EmplaceMoveCount)
{
    dense_hash_map<int, A> h;
    h.set_empty_key(0);

    h.emplace(1, 2);
    ASSERT_EQ(0, h[1].move_assign);
    ASSERT_EQ(0, h[1].move_ctor);
}

