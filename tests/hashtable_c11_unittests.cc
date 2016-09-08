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

TEST(HashtableMoveTest, OperatorInsertRValueCount)
{
    dense_hash_map<B, int, HashB> h;
    h.set_empty_key(B(0));

    B::reset();
    h[B(1)] = 1;

    std::cout << B() << std::endl;
    // without operator[](const K&): copy_ctor=2 copy_assign=0 move_ctor=1 move_assign=0
    // now:    copy_ctor=0 copy_assign=0 move_ctor=1 move_assign=0
    ASSERT_EQ(0, B::copy_ctor);
    ASSERT_EQ(0, B::copy_assign);
    ASSERT_EQ(1, B::move_ctor);
    ASSERT_EQ(0, B::move_assign);
}

TEST(HashtableMoveTest, EraseConstIterator)
{
    dense_hash_map<int, int> h;
    h.set_empty_key(0);
    h.set_deleted_key(-1);

    h[1] = 1;
    const auto it = h.begin();
    ASSERT_TRUE(h.end() == h.erase(it));
    ASSERT_EQ(0, (int)h.size());
}

TEST(HashtableMoveTest, EraseRange)
{
    dense_hash_map<int, int> h;
    h.set_empty_key(0);
    h.set_deleted_key(-1);

    for (int i = 0; i < 10; ++i)
        h[i + 1] = i;

    auto it = h.begin();
    std::advance(it, 2);

    auto it2 = h.begin();
    std::advance(it2, 8);

    auto nextit = h.erase(it, it2);
    ASSERT_FALSE(h.end() == nextit);
    nextit = h.erase(nextit);
    ASSERT_FALSE(h.end() == nextit);
    ASSERT_TRUE(h.end() == h.erase(nextit));

    ASSERT_FALSE(h.end() == h.erase(h.begin()));
    ASSERT_TRUE(h.end() == h.erase(h.begin()));

    ASSERT_TRUE(h.empty());
}

TEST(HashtableMoveTest, EraseRangeEntireMap)
{
    dense_hash_map<int, int> h;
    h.set_empty_key(0);
    h.set_deleted_key(-1);

    for (int i = 0; i < 10; ++i)
        h[i + 1] = i;

    ASSERT_TRUE(h.end() == h.erase(h.begin(), h.end()));
    ASSERT_TRUE(h.empty());
}

TEST(HashtableMoveTest, EraseNextElementReturned)
{
    dense_hash_map<int, int> h;
    h.set_empty_key(0);
    h.set_deleted_key(-1);

    h[1] = 1;
    h[2] = 2;

    int first = h.begin()->first;
    int second = first == 1 ? 2 : 1;

    ASSERT_EQ(second, h.erase(h.begin())->first); // second element is returned when erasing the first one
    ASSERT_TRUE(h.end() == h.erase(h.begin())); // end() is returned when erasing the second and last element
}

TEST(HashtableMoveTest, EraseNumberOfElementsDeleted)
{
    dense_hash_map<int, int> h;
    h.set_empty_key(0);
    h.set_deleted_key(-1);

    h[1] = 1;
    h[2] = 2;

    ASSERT_EQ(0, (int)h.erase(3));
    ASSERT_EQ(1, (int)h.erase(1));
    ASSERT_EQ(1, (int)h.erase(2));
    ASSERT_EQ(0, (int)h.erase(4));
    ASSERT_TRUE(h.empty());
}

TEST(HashtableMoveTest, CBeginCEnd)
{
    dense_hash_map<int, int> h;
    h.set_empty_key(0);
    h.set_deleted_key(-1);

    h[1] = 1;

    auto it = h.cbegin();
    ASSERT_EQ(1, it->first);

    std::advance(it, 1);
    ASSERT_TRUE(it == h.cend());

    using cit = dense_hash_map<int, int>::const_iterator;
    cit begin = h.begin();
    cit end = h.end();
    ASSERT_TRUE(begin == h.cbegin());
    ASSERT_TRUE(end == h.cend());
}
