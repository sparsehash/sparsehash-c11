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

    A(A&& a) { _i = a._i; move_ctor = a.move_ctor + 1; }
    A& operator=(A&& a) { _i = a._i; move_assign = a.move_assign + 1; return *this; }

    int _i = 0;
    int move_ctor = 0;
    int move_assign = 0;
};

bool operator==(const A& a1, const A& a2) { return a1._i == a2._i; }

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


struct B
{
    B() =default;
    B(int i) noexcept : _i(i) {}

    B(const B& b) { _i = b._i; ++copy_ctor;; }
    B& operator=(const B& b) { _i = b._i; ++copy_assign; return *this; }

    B(B&& b) { _i = b._i; ++move_ctor; }
    B& operator=(B&& b) { _i = b._i; ++move_assign; return *this; }

    static void reset()
    {
        copy_ctor = 0;
        copy_assign = 0;
        move_ctor = 0;
        move_assign = 0;
    }

    int _i = 0;

    static int copy_ctor;
    static int copy_assign;
    static int move_ctor;
    static int move_assign;
};

int B::copy_ctor = 0;
int B::copy_assign = 0;
int B::move_ctor = 0;
int B::move_assign = 0;

std::ostream& operator<<(std::ostream& os, const B& b)
{
    return os << b._i << " copy_ctor=" << b.copy_ctor << " copy_assign=" << b.copy_assign <<
                         " move_ctor=" << b.move_ctor << " move_assign=" << b.move_assign;
}

bool operator==(const B& b1, const B& b2) { return b1._i == b2._i; }

struct HashB
{
    std::size_t operator()(const B& b) const { return std::hash<int>()(b._i); }
};

TEST(HashtableMoveTest, EmplaceKeyMoveCount)
{
    dense_hash_map<B, int, HashB> h;
    h.set_empty_key(B(0));

    B::reset();
    auto p = h.emplace(1, 2);
    ASSERT_EQ(0, p.first->first.copy_ctor);
    ASSERT_EQ(0, p.first->first.copy_assign);
    ASSERT_EQ(0, p.first->first.move_ctor);
    ASSERT_EQ(0, p.first->first.move_assign);
}

TEST(HashtableMoveTest, InsertKeyRValueCount)
{
    dense_hash_map<B, int, HashB> h;
    h.set_empty_key(B(0));

    B::reset();
    auto p = h.insert(std::make_pair(B(2), 2));
    std::cout << p.first->first << std::endl;
    ASSERT_EQ(0, p.first->first.copy_ctor);
    ASSERT_EQ(0, p.first->first.copy_assign);
}

TEST(HashtableMoveTest, InsertKeyMovedCount)
{
    dense_hash_map<B, int, HashB> h;
    h.set_empty_key(B(0));

    B::reset();
    auto m = std::make_pair(B(2), 2);
    auto p = h.insert(std::move(m));

    std::cout << p.first->first << std::endl;
    ASSERT_EQ(0, p.first->first.copy_ctor);
    ASSERT_EQ(0, p.first->first.copy_assign);
}

TEST(HashtableMoveTest, InsertValueRValueCount)
{
    dense_hash_map<int, B> h;
    h.set_empty_key(0);

    B::reset();
    auto p = h.insert(std::make_pair(2, B(2)));

    std::cout << p.first->second << std::endl;
    ASSERT_EQ(0, p.first->second.copy_ctor);
    ASSERT_EQ(0, p.first->second.copy_assign);
}
TEST(HashtableMoveTest, InsertValueMovedCount)
{
    dense_hash_map<int, B> h;
    h.set_empty_key(0);

    B::reset();
    auto m = std::make_pair(2, B(2));
    auto p = h.insert(std::move(m));

    std::cout << p.first->second << std::endl;
    ASSERT_EQ(0, p.first->second.copy_ctor);
    ASSERT_EQ(0, p.first->second.copy_assign);
}

