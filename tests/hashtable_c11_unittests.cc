#include <sparsehash/dense_hash_map>
#include <unordered_map>

#include "fixture_unittests.h"
#include "hashtable_test_interface.h"
#include "gtest/gtest.h"
#include <unordered_map>
#include <unordered_set>
#include <chrono>

using google::dense_hash_map;
using google::dense_hash_set;

namespace sparsehash_internal = google::sparsehash_internal;

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

TEST(DenseHashMapMoveTest, Insert_RValue)
{
    dense_hash_map<int, std::unique_ptr<int>> h;
    h.set_empty_key(0);
    h.set_deleted_key(-1);

    auto p1 = std::make_pair(5, make_unique<int>(1234));
    auto p = h.insert(std::move(p1));
    ASSERT_EQ(true, p.second);
    ASSERT_EQ(5, p.first->first);
    ASSERT_EQ(1234, *p.first->second);

    p = h.insert(std::make_pair(10, make_unique<int>(5678)));
    ASSERT_EQ(true, p.second);
    ASSERT_EQ(10, p.first->first);
    ASSERT_EQ(5678, *p.first->second);

    ASSERT_EQ(2, (int)h.size());
}

TEST(DenseHashMapMoveTest, Emplace)
{
    dense_hash_map<int, std::unique_ptr<int>> h;
    h.set_empty_key(0);
    h.set_deleted_key(-1);

    auto p = h.emplace(5, make_unique<int>(1234));
    ASSERT_EQ(true, p.second);
    ASSERT_EQ(5, p.first->first);
    ASSERT_EQ(1234, *p.first->second);

    p = h.emplace(10, make_unique<int>(5678));
    ASSERT_EQ(true, p.second);
    ASSERT_EQ(10, p.first->first);
    ASSERT_EQ(5678, *p.first->second);

    ASSERT_EQ(2, (int)h.size());

    ASSERT_TRUE(h.emplace(11, make_unique<int>(1)).second);
    ASSERT_FALSE(h.emplace(11, make_unique<int>(1)).second);
    ASSERT_TRUE(h.emplace(12, make_unique<int>(1)).second);
    ASSERT_FALSE(h.emplace(12, make_unique<int>(1)).second);
}

TEST(DenseHashMapMoveTest, EmplaceHint)
{
    dense_hash_map<int, int> h;
    h.set_empty_key(0);

    h[1] = 1;
    h[3] = 3;
    h[5] = 5;

    ASSERT_FALSE(h.emplace_hint(h.find(1), 1, 0).second);
    ASSERT_FALSE(h.emplace_hint(h.find(3), 1, 0).second);
    ASSERT_FALSE(h.emplace_hint(h.end(), 1, 0).second);
    ASSERT_EQ(3, (int)h.size());

    ASSERT_TRUE(h.emplace_hint(h.find(1), 2, 0).second);
    ASSERT_TRUE(h.emplace_hint(h.find(3), 4, 0).second);
    ASSERT_TRUE(h.emplace_hint(h.end(), 6, 0).second);
    ASSERT_EQ(6, (int)h.size());
}

#ifndef _SPARSEHASH_CI_TESTING_ // CI and speed tests don't mix
TEST(DenseHashMapMoveTest, EmplaceHint_SpeedComparison)
{
    static const int Elements = 1e6;
    static const int Duplicates = 5;

    std::vector<int> v(Elements * Duplicates);

    for (int i = 0; i < Elements * Duplicates; i += Duplicates)
    {
        auto r = std::rand();

        for (int j = 0; j < Duplicates; ++j)
            v[i + j] = r;
    }

    std::sort(std::begin(v), std::end(v));

    auto start = std::chrono::system_clock::now();

    {
        dense_hash_map<int, int> h;
        h.set_empty_key(-1);

        for (int i = 0; i < Elements; ++i)
            h.emplace(v[i], 0);
    }

    auto end = std::chrono::system_clock::now();
    auto emplace_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    start = std::chrono::system_clock::now();

    {
        dense_hash_map<int, int> h;
        h.set_empty_key(-1);

        auto hint = h.begin();
        for (int i = 0; i < Elements; ++i)
            hint = h.emplace_hint(hint, v[i], 0).first;
    }

    end = std::chrono::system_clock::now();
    auto emplace_hint_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    ASSERT_LE(emplace_hint_time, emplace_time);
}
#endif  // _SPARSEHASH_CI_TESTING_

struct A
{
    A() =default;
    A(int i) noexcept : _i(i) {}

    A(const A& a) { _i = a._i; ++copy_ctor;; }
    A& operator=(const A& a) { _i = a._i; ++copy_assign; return *this; }

    A(A&& a) { _i = a._i; ++move_ctor; }
    A& operator=(A&& a) { _i = a._i; ++move_assign; return *this; }

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

int A::copy_ctor = 0;
int A::copy_assign = 0;
int A::move_ctor = 0;
int A::move_assign = 0;

std::ostream& operator<<(std::ostream& os, const A& a)
{
    return os << a._i << " copy_ctor=" << a.copy_ctor << " copy_assign=" << a.copy_assign <<
                         " move_ctor=" << a.move_ctor << " move_assign=" << a.move_assign;
}

bool operator==(const A& a1, const A& a2) { return a1._i == a2._i; }

struct HashA
{
    std::size_t operator()(const A& a) const { return std::hash<int>()(a._i); }
};


TEST(DenseHashMapMoveTest, MoveConstructor)
{
    dense_hash_map<int, A> h(10);
    h.set_empty_key(0);

    h.emplace(1, 2);
    h.emplace(2, 3);
    h.emplace(3, 4);
    A::reset();

    dense_hash_map<int, A> h2(std::move(h));

    ASSERT_EQ(3, (int)h2.size());

    for (auto&& p : h2)
        std::cout << p.first << std::endl;

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(0, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(DenseHashMapMoveTest, MoveAssignment)
{
    dense_hash_map<int, A> h(10), h2;
    h.set_empty_key(0);

    h.emplace(1, 2);
    h.emplace(2, 3);
    h.emplace(3, 4);
    A::reset();

    h2 = std::move(h);

    ASSERT_EQ(3, (int)h2.size());

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(0, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(DenseHashMapMoveTest, InsertRValue_ValueMoveCount)
{
    dense_hash_map<int, A> h(10);
    h.set_empty_key(0);

    A::reset();
    h.insert(std::make_pair(5, A()));

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(2, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(DenseHashMapMoveTest, InsertMoved_ValueMoveCount)
{
    dense_hash_map<int, A> h(10);
    h.set_empty_key(0);

    auto p = std::make_pair(5, A());

    A::reset();
    h.insert(std::move(p));

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(1, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(DenseHashMapMoveTest, Emplace_ValueMoveCount)
{
    dense_hash_map<int, A> h;
    h.set_empty_key(0);

    A::reset();
    h.emplace(1, 2);

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(0, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(DenseHashMapMoveTest, InsertRValue_KeyMoveCount)
{
    dense_hash_map<A, int, HashA> h;
    h.set_empty_key(A(0));

    A::reset();
    h.insert(std::make_pair(A(2), 2));

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(2, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(DenseHashMapMoveTest, InsertMoved_KeyMoveCount)
{
    dense_hash_map<A, int, HashA> h;
    h.set_empty_key(A(0));

    auto m = std::make_pair(A(2), 2);
    A::reset();
    h.insert(std::move(m));

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(1, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(DenseHashMapMoveTest, Emplace_KeyMoveCount)
{
    dense_hash_map<A, int, HashA> h;
    h.set_empty_key(A(0));

    A::reset();
    h.emplace(1, 2);

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(0, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(DenseHashMapMoveTest, OperatorSqBck_InsertRValue_KeyMoveCount)
{
    dense_hash_map<A, int, HashA> h;
    h.set_empty_key(A(0));

    A::reset();
    h[A(1)] = 1;

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(1, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(DenseHashMapMoveTest, OperatorSqBck_InsertMoved_ValueMoveCount)
{
    dense_hash_map<int, A> h;
    h.set_empty_key(0);

    A::reset();
    h[1] = A(1);

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(1, A::move_ctor);
    ASSERT_EQ(1, A::move_assign);
}

TEST(DenseHashMapMoveTest, EraseConstIterator)
{
    dense_hash_map<int, int> h;
    h.set_empty_key(0);
    h.set_deleted_key(-1);

    h[1] = 1;
    const auto it = h.begin();
    ASSERT_TRUE(h.end() == h.erase(it));
    ASSERT_EQ(0, (int)h.size());
}

TEST(DenseHashMapMoveTest, EraseRange)
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

TEST(DenseHashMapMoveTest, EraseRangeEntireMap)
{
    dense_hash_map<int, int> h;
    h.set_empty_key(0);
    h.set_deleted_key(-1);

    for (int i = 0; i < 10; ++i)
        h[i + 1] = i;

    ASSERT_TRUE(h.end() == h.erase(h.begin(), h.end()));
    ASSERT_TRUE(h.empty());
}

TEST(DenseHashMapMoveTest, EraseNextElementReturned)
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

TEST(DenseHashMapMoveTest, EraseNumberOfElementsDeleted)
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

TEST(DenseHashMapMoveTest, CBeginCEnd)
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

TEST(DenseHashSetIfaceTest, BucketBeginEnd)
{
    dense_hash_set<int> h;
    h.set_empty_key(0);
    h.set_deleted_key(-1);

    EXPECT_TRUE(h.begin(0) == h.end(0)) << "empty set";
    EXPECT_TRUE(h.begin(10) == h.end(10)) << "empty set";
    EXPECT_TRUE(h.cbegin(0) == h.cend(0)) << "empty set";
    EXPECT_TRUE(h.cbegin(10) == h.cend(10)) << "empty set";

    h.insert(1);
    const auto n1 = h.bucket(1);
    EXPECT_FALSE(h.begin(n1) == h.end(n1)) << "singleton set";
    EXPECT_EQ(1, std::distance(h.begin(n1), h.end(n1))) << "singleton set";
    EXPECT_FALSE(h.cbegin(n1) == h.cend(n1)) << "singleton set";
    EXPECT_EQ(1, *h.cbegin(n1)) << "singleton set";

    h.insert(2);
    const auto n2 = h.bucket(2);
    EXPECT_FALSE(h.begin(n1) == h.end(n1)) << "two elements set";
    EXPECT_EQ(1, std::distance(h.begin(n1), h.end(n1))) << "two elements set";
    EXPECT_FALSE(h.begin(n2) == h.end(n2)) << "two elements set";
    EXPECT_EQ(1, std::distance(h.begin(n2), h.end(n2))) << "two elements set";
    EXPECT_EQ(1, std::distance(h.cbegin(n1), h.cend(n1))) << "two elements set";
    EXPECT_EQ(1, std::distance(h.cbegin(n2), h.cend(n2))) << "two elements set";
    EXPECT_EQ(1, *h.cbegin(n1)) << "two elements set";
    EXPECT_EQ(2, *h.cbegin(n2)) << "two elements set";
}

TEST(DenseHashMapIfaceTest, BucketBeginEnd)
{
    dense_hash_map<int, int> h;
    h.set_empty_key(0);
    h.set_deleted_key(-1);

    EXPECT_TRUE(h.begin(0) == h.end(0)) << "empty map";
    EXPECT_TRUE(h.begin(10) == h.end(10)) << "empty map";
    EXPECT_TRUE(h.cbegin(0) == h.cend(0)) << "empty map";
    EXPECT_TRUE(h.cbegin(10) == h.cend(10)) << "empty map";

    h.emplace(1, 10);
    const auto n1 = h.bucket(1);
    EXPECT_FALSE(h.begin(n1) == h.end(n1)) << "singleton map";
    EXPECT_EQ(1, std::distance(h.begin(n1), h.end(n1))) << "singleton map";
    EXPECT_FALSE(h.cbegin(n1) == h.cend(n1)) << "singleton map";
    EXPECT_EQ(10, h.cbegin(n1)->second) << "singleton map";

    h.emplace(2, 20);
    const auto n2 = h.bucket(2);
    EXPECT_FALSE(h.begin(n1) == h.end(n1)) << "two elements map";
    EXPECT_EQ(1, std::distance(h.begin(n1), h.end(n1))) << "two elements map";
    EXPECT_FALSE(h.begin(n2) == h.end(n2)) << "two elements map";
    EXPECT_EQ(1, std::distance(h.begin(n2), h.end(n2))) << "two elements map";
    EXPECT_EQ(1, std::distance(h.cbegin(n1), h.cend(n1))) << "two elements map";
    EXPECT_EQ(1, std::distance(h.cbegin(n2), h.cend(n2))) << "two elements map";
    EXPECT_EQ(10, h.cbegin(n1)->second) << "two elements map";
    EXPECT_EQ(20, h.cbegin(n2)->second) << "two elements map";
}

TEST(DenseHashSetMoveTest, MoveConstructor)
{
    dense_hash_set<A, HashA> h;
    h.set_empty_key(A(0));

    const int Elements = 100;
    for (int i = 1; i <= Elements; ++i)
        h.emplace(i);

    A::reset();
    dense_hash_set<A, HashA> h2(std::move(h));

    ASSERT_EQ(Elements, (int)h2.size());

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(2, A::move_ctor); // swap() of empty & deleted key
    ASSERT_EQ(4, A::move_assign);
}

TEST(DenseHashSetMoveTest, MoveAssignment)
{
    dense_hash_set<A, HashA> h, h2;
    h.set_empty_key(A(0));

    const int Elements = 100;
    for (int i = 1; i <= Elements; ++i)
        h.emplace(i);

    A::reset();
    h2 = std::move(h);

    ASSERT_EQ(Elements, (int)h2.size());

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(2, A::move_ctor); // swap() of empty & deleted key
    ASSERT_EQ(4, A::move_assign);
}

TEST(DenseHashSetMoveTest, Insert)
{
    dense_hash_set<A, HashA> h;
    h.set_empty_key(A(0));

    A::reset();
    h.insert(A(1));

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(1, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(DenseHashSetMoveTest, Emplace)
{
    dense_hash_set<A, HashA> h;
    h.set_empty_key(A(0));

    A::reset();
    h.emplace(1);

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(0, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(DenseHashSetIfaceTest, Insert)
{
    dense_hash_set<int> h;
    h.set_empty_key(0);

    const int k1 = 10;
    int k2 = 20;
    std::vector<int> v { 5, 15, 25, 35 };

    EXPECT_TRUE(h.insert(k1).second);
    EXPECT_TRUE(h.insert(k2).second);
    h.insert(h.begin(), 2);
    h.insert(h.begin(), std::move(3));
    h.insert(v.begin(), v.end());
    h.insert({11, 21, 31});
    EXPECT_EQ(11ul, h.size());
}

TEST(DenseHashMapIfaceTest, Insert)
{
    dense_hash_map<int, int> h;
    h.set_empty_key(0);

    const std::pair<const int, int> k1 { 10, 100 };
    std::pair<const int, int> k2 { 20, 200 };
    const std::pair<const int, int> k3 { 30, 300 };
    std::vector<std::pair<const int, int>> v { {5, 50}, {15, 150} };

    EXPECT_TRUE(h.insert(k1).second);
    EXPECT_TRUE(h.insert(k2).second);
    h.insert(h.begin(), k3);
    h.insert(h.begin(), std::pair<const int, int>{2, 20});
    h.insert(v.begin(), v.end());
    h.insert({{11, 110}, {21, 210}, {31, 310}});
    EXPECT_EQ(9ul, h.size());
}
