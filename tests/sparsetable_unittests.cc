// Copyright (c) 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Since sparsetable is templatized, it's important that we test every
// function in every class in this file -- not just to see if it
// works, but even if it compiles.

#include <memory>     // for allocator
#include <algorithm>  // for swap
#include <string>
#include <vector>
#include <cstdio>
#include <cstdint>
#include "gtest/gtest.h"
#include "gtest/gmock.h"  // for ElementsAre,ElementsAreArray and ContainerEq
#include "sparsehash/sparsetable"

using google::sparsetable;
using google::DEFAULT_SPARSEGROUP_SIZE;

using namespace testing;

template <class T>
T const& constant(const T& v) {
  return v;
}

template <class T>
std::vector<typename T::value_type> ToVector(T start, T end) {
  return std::vector<typename T::value_type>(start, end);
}

// Test sparsetable with a POD type, int.
TEST(Sparsetable, Int) {
  sparsetable<int> x(7), y(70), z;
  x.set(4, 10);
  y.set(12, -12);
  y.set(47, -47);
  y.set(48, -48);
  y.set(49, -49);

  const sparsetable<int> constx(x);
  const sparsetable<int> consty(y);

  // ----------------------------------------------------------------------
  // Test the plain iterators

  ASSERT_THAT(ToVector(x.begin(), x.end()), ElementsAre(0, 0, 0, 0, 10, 0, 0));

  ASSERT_THAT(ToVector(constx.begin(), constx.end()),
              ElementsAre(0, 0, 0, 0, 10, 0, 0));

  ASSERT_THAT(ToVector(x.rbegin(), x.rend()),
              ElementsAre(0, 0, 10, 0, 0, 0, 0));

  ASSERT_THAT(ToVector(constx.rbegin(), constx.rend()),
              ElementsAre(0, 0, 10, 0, 0, 0, 0));

  for (auto it = z.begin(); it != z.end(); ++it) {
    FAIL() << "z should be empty";
  }

  // ----------------------------------------------------------------------
  // array version
  ASSERT_EQ(0, static_cast<int>(x[3]));
  ASSERT_EQ(10, static_cast<int>(x[4]));
  ASSERT_EQ(0, static_cast<int>(x[5]));

  {
    sparsetable<int>::iterator it;  // non-const version
    ASSERT_EQ(10, static_cast<int>(x.begin()[4]));
    it = x.begin() + 4;  // should point to the non-zero value
    ASSERT_EQ(10, static_cast<int>(*it));
    it--;
    --it;
    it += 5;
    it -= 2;
    it++;
    ++it;
    it = it - 3;
    it = 1 + it;  // now at 5
    ASSERT_EQ(0, static_cast<int>(it[-2]));
    ASSERT_EQ(10, static_cast<int>(it[-1]));
    *it = 55;
    ASSERT_EQ(55, static_cast<int>(it[0]));
    ASSERT_EQ(55, static_cast<int>(*it));
    int* x6 = &(it[1]);
    *x6 = 66;
    ASSERT_EQ(66, static_cast<int>(*(it + 1)));

    // Let's test comparators as well
    ASSERT_TRUE(it == it);
    ASSERT_TRUE(!(it != it));
    ASSERT_TRUE(!(it < it));
    ASSERT_TRUE(!(it > it));
    ASSERT_TRUE(it <= it);
    ASSERT_TRUE(it >= it);

    sparsetable<int>::iterator it_minus_1 = it - 1;
    ASSERT_TRUE(!(it == it_minus_1));
    ASSERT_TRUE(it != it_minus_1);
    ASSERT_TRUE(!(it < it_minus_1));
    ASSERT_TRUE(it > it_minus_1);
    ASSERT_TRUE(!(it <= it_minus_1));
    ASSERT_TRUE(it >= it_minus_1);
    ASSERT_TRUE(!(it_minus_1 == it));
    ASSERT_TRUE(it_minus_1 != it);
    ASSERT_TRUE(it_minus_1 < it);
    ASSERT_TRUE(!(it_minus_1 > it));
    ASSERT_TRUE(it_minus_1 <= it);
    ASSERT_TRUE(!(it_minus_1 >= it));

    sparsetable<int>::iterator it_plus_1 = it + 1;
    ASSERT_TRUE(!(it == it_plus_1));
    ASSERT_TRUE(it != it_plus_1);
    ASSERT_TRUE(it < it_plus_1);
    ASSERT_TRUE(!(it > it_plus_1));
    ASSERT_TRUE(it <= it_plus_1);
    ASSERT_TRUE(!(it >= it_plus_1));
    ASSERT_TRUE(!(it_plus_1 == it));
    ASSERT_TRUE(it_plus_1 != it);
    ASSERT_TRUE(!(it_plus_1 < it));
    ASSERT_TRUE(it_plus_1 > it);
    ASSERT_TRUE(!(it_plus_1 <= it));
    ASSERT_TRUE(it_plus_1 >= it);
  }
  {
    sparsetable<int>::const_iterator it;  // const version
    ASSERT_EQ(10, static_cast<int>(x.begin()[4]));
    it = x.begin() + 4;  // should point to the non-zero value
    ASSERT_EQ(10, static_cast<int>(*it));
    it--;
    --it;
    it += 5;
    it -= 2;
    it++;
    ++it;
    it = it - 3;
    it = 1 + it;  // now at 5

    ASSERT_EQ(0, it[-2]);
    ASSERT_EQ(10, it[-1]);
    ASSERT_EQ(55, *it);
    ASSERT_EQ(66, *(it + 1));
    // Let's test comparators as well
    ASSERT_TRUE(it == it);
    ASSERT_TRUE(!(it != it));
    ASSERT_TRUE(!(it < it));
    ASSERT_TRUE(!(it > it));
    ASSERT_TRUE(it <= it);
    ASSERT_TRUE(it >= it);

    sparsetable<int>::const_iterator it_minus_1 = it - 1;
    ASSERT_TRUE(!(it == it_minus_1));
    ASSERT_TRUE(it != it_minus_1);
    ASSERT_TRUE(!(it < it_minus_1));
    ASSERT_TRUE(it > it_minus_1);
    ASSERT_TRUE(!(it <= it_minus_1));
    ASSERT_TRUE(it >= it_minus_1);
    ASSERT_TRUE(!(it_minus_1 == it));
    ASSERT_TRUE(it_minus_1 != it);
    ASSERT_TRUE(it_minus_1 < it);
    ASSERT_TRUE(!(it_minus_1 > it));
    ASSERT_TRUE(it_minus_1 <= it);
    ASSERT_TRUE(!(it_minus_1 >= it));

    sparsetable<int>::const_iterator it_plus_1 = it + 1;
    ASSERT_TRUE(!(it == it_plus_1));
    ASSERT_TRUE(it != it_plus_1);
    ASSERT_TRUE(it < it_plus_1);
    ASSERT_TRUE(!(it > it_plus_1));
    ASSERT_TRUE(it <= it_plus_1);
    ASSERT_TRUE(!(it >= it_plus_1));
    ASSERT_TRUE(!(it_plus_1 == it));
    ASSERT_TRUE(it_plus_1 != it);
    ASSERT_TRUE(!(it_plus_1 < it));
    ASSERT_TRUE(it_plus_1 > it);
    ASSERT_TRUE(!(it_plus_1 <= it));
    ASSERT_TRUE(it_plus_1 >= it);
  }

  ASSERT_TRUE(x.begin() == x.begin() + 1 - 1);
  ASSERT_TRUE(x.begin() < x.end());
  ASSERT_FALSE(z.begin() < z.end());  // Wrong in original tests
  ASSERT_TRUE(z.begin() <= z.end());
  ASSERT_TRUE(z.begin() == z.end());

  // ----------------------------------------------------------------------
  // Test the non-empty iterators
  ASSERT_THAT(ToVector(x.nonempty_begin(), x.nonempty_end()),
              ElementsAre(10, 55, 66));

  ASSERT_THAT(
      ToVector(constant(y).nonempty_begin(), constant(y).nonempty_end()),
      ElementsAre(-12, -47, -48, -49));

  ASSERT_THAT(ToVector(y.nonempty_rbegin(), y.nonempty_rend()),
              ElementsAre(-49, -48, -47, -12));

  ASSERT_THAT(ToVector(consty.nonempty_rbegin(), consty.nonempty_rend()),
              ElementsAre(-49, -48, -47, -12));

  for (sparsetable<int>::nonempty_iterator it = z.nonempty_begin();
       it != z.nonempty_end(); ++it) {
    FAIL() << "z should be empty";
  }

  {
    sparsetable<int>::nonempty_iterator it;  // non-const version
    ASSERT_EQ(-12, *y.nonempty_begin());
    ASSERT_EQ(10, *x.nonempty_begin());
    it = x.nonempty_begin();
    ++it;  // should be at end
    --it;
    ASSERT_EQ(10, *it++);
    it--;
    ASSERT_EQ(10, *it++);
  }
  {
    sparsetable<int>::const_nonempty_iterator it;  // const version
    ASSERT_EQ(-12, *y.nonempty_begin());
    ASSERT_EQ(10, *x.nonempty_begin());
    it = x.nonempty_begin();
    ++it;  // should be at end
    --it;
    ASSERT_EQ(10, *it++);
    it--;
    ASSERT_EQ(10, *it++);
  }

  ASSERT_TRUE(x.begin() == x.begin() + 1 - 1);
  ASSERT_TRUE(z.begin() == z.end());

  // ----------------------------------------------------------------------
  // Test the non-empty iterators get_pos function

  sparsetable<unsigned int> gp(100);

  for (std::size_t i = 0; i < 100; i += 9) gp.set(i, i);

  {
    std::vector<unsigned int> v;
    for (auto it = constant(gp).nonempty_begin();
         it != constant(gp).nonempty_end(); ++it)
      v.emplace_back(static_cast<unsigned int>(gp.get_pos(it)));

    ASSERT_THAT(
        v, ElementsAreArray({0, 9, 18, 27, 36, 45, 54, 63, 72, 81, 90, 99}));
  }

  {
    std::vector<unsigned int> v;
    for (auto it = gp.nonempty_begin(); it != gp.nonempty_end(); ++it)
      v.emplace_back(static_cast<unsigned int>(gp.get_pos(it)));
    ASSERT_THAT(
        v, ElementsAreArray({0, 9, 18, 27, 36, 45, 54, 63, 72, 81, 90, 99}));
  }

  // ----------------------------------------------------------------------
  // Test sparsetable functions

  ASSERT_EQ(3UL, x.num_nonempty());
  ASSERT_EQ(7UL, x.size());

  ASSERT_EQ(4UL, y.num_nonempty());
  ASSERT_EQ(70UL, y.size());

  ASSERT_EQ(0UL, z.num_nonempty());
  ASSERT_EQ(0UL, z.size());

  y.resize(48);  // should get rid of 48 and 49
  y.resize(70);  // 48 and 49 should still be gone

  ASSERT_EQ(2UL, y.num_nonempty());
  ASSERT_EQ(70UL, y.size());

  ASSERT_EQ(-12, static_cast<int>(y[12]));
  ASSERT_EQ(-12, y.get(12));

  y.erase(12);

  ASSERT_EQ(1UL, y.num_nonempty());
  ASSERT_EQ(70UL, y.size());

  ASSERT_EQ(0, static_cast<int>(y[12]));
  ASSERT_EQ(0, y.get(12));

  swap(x, y);

  y.clear();
  // TODO: why does this fail?
  // ASSERT_EQ(y, z);

  y.resize(70);
  for (std::size_t i = 10; i < 40; ++i) y[i] = -i;
  y.erase(y.begin() + 15, y.begin() + 30);
  y.erase(y.begin() + 34);
  y.erase(12);
  y.resize(38);
  y.resize(10000);
  y[9898] = -9898;

  {
    std::vector<unsigned int> v;
    for (auto it = constant(y).begin(); it != constant(y).end(); ++it) {
      if (y.test(it)) v.emplace_back(static_cast<unsigned int>(it - y.begin()));
    }
    ASSERT_THAT(v, ElementsAreArray(
                       {10, 11, 13, 14, 30, 31, 32, 33, 35, 36, 37, 9898}));
  }
  ASSERT_EQ(12UL, y.num_nonempty());

  {
    std::vector<unsigned int> v;
    for (auto it = constant(y).get_iter(32); it != constant(y).nonempty_end();
         ++it)
      v.emplace_back(*it);
    ASSERT_THAT(v, ElementsAre(-32, -33, -35, -36, -37, -9898));
  }
  {
    std::vector<unsigned int> v;
    for (auto it = y.get_iter(32); it != y.nonempty_begin();)
      v.emplace_back(*--it);
    ASSERT_THAT(v, ElementsAre(-31, -30, -14, -13, -11, -10));
  }

  // ----------------------------------------------------------------------
  // Test I/O using deprecated read/write_metadata

  {
    auto fp = std::tmpfile();

    ASSERT_TRUE(fp) << "Can't open temp file";

    y.write_metadata(fp);  // only write meta-information
    y.write_nopointer_data(fp);
    std::rewind(fp);

    sparsetable<int> y2;
    y2.read_metadata(fp);
    y2.read_nopointer_data(fp);

    ASSERT_THAT(y, ContainerEq(y2));
    ASSERT_EQ(12UL, y2.num_nonempty());
  }

  // ----------------------------------------------------------------------
  // Also test I/O using serialize()/unserialize()

  {
    auto fp = std::tmpfile();

    ASSERT_TRUE(fp) << "Can't open temp file";

    y.serialize(sparsetable<int>::NopointerSerializer(), fp);

    std::rewind(fp);

    sparsetable<int> y2;
    y2.unserialize(sparsetable<int>::NopointerSerializer(), fp);

    ASSERT_THAT(y, ContainerEq(y2));
    ASSERT_EQ(12UL, y2.num_nonempty());
  }
}

// An instrumented allocator that keeps track of all calls to
// allocate/deallocate/construct/destroy. It stores the number of times
// they were called and the values they were called with. Such information is
// stored in the following global variables.

static size_t sum_allocate_bytes;
static size_t sum_deallocate_bytes;

void ResetAllocatorCounters() {
  sum_allocate_bytes = 0;
  sum_deallocate_bytes = 0;
}

template <class T>
class instrumented_allocator {
 public:
  typedef T value_type;
  typedef uint16_t size_type;
  typedef ptrdiff_t difference_type;

  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;

  instrumented_allocator() {}
  instrumented_allocator(const instrumented_allocator&) {}
  ~instrumented_allocator() {}

  pointer address(reference r) const { return &r; }
  const_pointer address(const_reference r) const { return &r; }

  pointer allocate(size_type n, const_pointer = 0) {
    sum_allocate_bytes += n * sizeof(value_type);
    return static_cast<pointer>(malloc(n * sizeof(value_type)));
  }
  void deallocate(pointer p, size_type n) {
    sum_deallocate_bytes += n * sizeof(value_type);
    free(p);
  }

  size_type max_size() const {
    return static_cast<size_type>(-1) / sizeof(value_type);
  }

  void construct(pointer p, const value_type& val) { new (p) value_type(val); }
  void destroy(pointer p) { p->~value_type(); }

  template <class U>
  explicit instrumented_allocator(const instrumented_allocator<U>&) {}

  template <class U>
  struct rebind {
    typedef instrumented_allocator<U> other;
  };

 private:
  void operator=(const instrumented_allocator&);
};

template <class T>
inline bool operator==(const instrumented_allocator<T>&,
                       const instrumented_allocator<T>&) {
  return true;
}

template <class T>
inline bool operator!=(const instrumented_allocator<T>&,
                       const instrumented_allocator<T>&) {
  return false;
}

// Test sparsetable with instrumented_allocator.
TEST(Sparsetable, Allocator) {
  ResetAllocatorCounters();

  // POD (int32) with instrumented_allocator.
  typedef sparsetable<int, DEFAULT_SPARSEGROUP_SIZE,
                      instrumented_allocator<int> > IntSparseTable;

  IntSparseTable* s1 = new IntSparseTable(10000);
  ASSERT_GT(sum_allocate_bytes, 0UL);
  for (int i = 0; i < 10000; ++i) {
    s1->set(i, 0);
  }
  ASSERT_GE(sum_allocate_bytes, 10000UL * sizeof(int));
  ResetAllocatorCounters();
  delete s1;
  ASSERT_GE(sum_deallocate_bytes, 10000UL * sizeof(int));

  IntSparseTable* s2 = new IntSparseTable(1000);
  IntSparseTable* s3 = new IntSparseTable(1000);

  for (int i = 0; i < 1000; ++i) {
    s2->set(i, 0);
    s3->set(i, 0);
  }
  ASSERT_GE(sum_allocate_bytes, 2000UL * sizeof(int));

  ResetAllocatorCounters();
  s3->clear();
  ASSERT_GE(sum_deallocate_bytes, 1000UL * sizeof(int));

  ResetAllocatorCounters();
  s2->swap(*s3);  // s2 is empty after the swap
  s2->clear();
  ASSERT_LT(sum_deallocate_bytes, 1000UL * sizeof(int));
  for (int i = 0; i < s3->size(); ++i) {
    s3->erase(i);
  }
  ASSERT_GE(sum_deallocate_bytes, 1000UL * sizeof(int));
  delete s2;
  delete s3;

  // POD (int) with default allocator.
  sparsetable<int> x, y;
  for (int s = 1000; s <= 40000; s += 1000) {
    x.resize(s);
    for (int i = 0; i < s; ++i) {
      x.set(i, i + 1);
    }
    y = x;
    for (int i = 0; i < s; ++i) {
      y.erase(i);
    }
    y.swap(x);
  }
  ASSERT_EQ(0UL, x.num_nonempty());
  ASSERT_EQ(1, static_cast<int>(y[0]));
  ASSERT_EQ(40000, static_cast<int>(y[39999]));
  y.clear();

  // POD (int) with std allocator.
  sparsetable<int, DEFAULT_SPARSEGROUP_SIZE, std::allocator<int> > u, v;
  for (int s = 1000; s <= 40000; s += 1000) {
    u.resize(s);
    for (int i = 0; i < s; ++i) {
      u.set(i, i + 1);
    }
    v = u;
    for (int i = 0; i < s; ++i) {
      v.erase(i);
    }
    v.swap(u);
  }
  ASSERT_EQ(0UL, u.num_nonempty());
  ASSERT_EQ(1, static_cast<int>(v[0]));
  ASSERT_EQ(40000, static_cast<int>(v[39999]));
  v.clear();

  // Non-POD (string) with default allocator.
  sparsetable<std::string> a, b;
  for (int s = 1000; s <= 40000; s += 1000) {
    a.resize(s);
    for (int i = 0; i < s; ++i) {
      a.set(i, "aa");
    }
    b = a;
    for (int i = 0; i < s; ++i) {
      b.erase(i);
    }
    b.swap(a);
  }
  ASSERT_EQ(0UL, a.num_nonempty());
  ASSERT_STREQ("aa", b.get(0).c_str());
  ASSERT_STREQ("aa", b.get(39999).c_str());
  b.clear();
}