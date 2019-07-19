#pragma once

#include <cmath>
#include <cstddef>  // for size_t
#include <cstdlib>
#include <cstring>
#include <cstdint>  // for uintptr_t
#include <string>
#include <type_traits>

#include <sparsehash/sparsetable>
#include "hashtable_test_interface.h"

#include "gtest/gtest.h"

using std::pair;
using std::string;
using std::swap;
using google::dense_hash_map;
using google::dense_hash_set;
using google::sparse_hash_map;
using google::sparse_hash_set;
using google::sparsetable;
using google::HashtableInterface_SparseHashMap;
using google::HashtableInterface_SparseHashSet;
using google::HashtableInterface_SparseHashtable;
using google::HashtableInterface_DenseHashMap;
using google::HashtableInterface_DenseHashSet;
using google::HashtableInterface_DenseHashtable;
namespace sparsehash_internal = google::sparsehash_internal;

using namespace testing;

typedef unsigned char uint8;

#ifdef _MSC_VER
    // 4503 keeps quiet errors "decorated name length exceeded"
    #pragma warning(disable : 4503)
#endif

// #ifdef _MSC_VER
// // Below, we purposefully test having a very small allocator size.
// // This causes some "type conversion too small" errors when using this
// // allocator with sparsetable buckets.  We're testing to make sure we
// // handle that situation ok, so we don't need the compiler warnings.
// #pragma warning(disable : 4244)
// #endif

// Used as a value in some of the hashtable tests.  It's just some
// arbitrary user-defined type with non-trivial memory management.
struct ValueType {
 public:
  ValueType() : s_(kDefault) {}
  ValueType(const char* init_s) : s_(kDefault) { set_s(init_s); }
  ~ValueType() { set_s(NULL); }
  ValueType(const ValueType& that) : s_(kDefault) { operator=(that); }
  void operator=(const ValueType& that) { set_s(that.s_); }
  bool operator==(const ValueType& that) const {
    return strcmp(this->s(), that.s()) == 0;
  }
  void set_s(const char* new_s) {
    if (s_ != kDefault) free(const_cast<char*>(s_));
    s_ = (new_s == NULL ? kDefault : reinterpret_cast<char*>(strdup(new_s)));
  }
  const char* s() const { return s_; }

 private:
  const char* s_;
  static const char* const kDefault;
};

// This is used by the low-level sparse/dense_hashtable classes,
// which support the most general relationship between keys and
// values: the key is derived from the value through some arbitrary
// function.  (For classes like sparse_hash_map, the 'value' is a
// key/data pair, and the function to derive the key is
// FirstElementOfPair.)  KeyToValue is the inverse of this function,
// so GetKey(KeyToValue(key)) == key.  To keep the tests a bit
// simpler, we've chosen to make the key and value actually be the
// same type, which is why we need only one template argument for the
// types, rather than two (one for the key and one for the value).
template <class KeyAndValueT, class KeyToValue>
struct SetKey {
  void operator()(KeyAndValueT* value, const KeyAndValueT& new_key) const {
    *value = KeyToValue()(new_key);
  }
  void operator()(KeyAndValueT* value, const KeyAndValueT& new_key, bool) const {
    new (value) KeyAndValueT();
    *value = KeyToValue()(new_key);
  }
};

// A hash function that keeps track of how often it's called.  We use
// a simple djb-hash so we don't depend on how STL hashes.  We use
// this same method to do the key-comparison, so we can keep track
// of comparison-counts too.
struct Hasher {
  explicit Hasher(int i = 0) : id_(i), num_hashes_(0), num_compares_(0) {}
  int id() const { return id_; }
  int num_hashes() const { return num_hashes_; }
  int num_compares() const { return num_compares_; }

  size_t operator()(int a) const {
    num_hashes_++;
    return static_cast<size_t>(a);
  }
  size_t operator()(const char* a) const {
    num_hashes_++;
    size_t hash = 0;
    for (size_t i = 0; a[i]; i++) hash = 33 * hash + a[i];
    return hash;
  }
  size_t operator()(const string& a) const {
    num_hashes_++;
    size_t hash = 0;
    for (size_t i = 0; i < a.length(); i++) hash = 33 * hash + a[i];
    return hash;
  }
  size_t operator()(const int* a) const {
    num_hashes_++;
    return static_cast<size_t>(reinterpret_cast<uintptr_t>(a));
  }
  bool operator()(int a, int b) const {
    num_compares_++;
    return a == b;
  }
  bool operator()(const string& a, const string& b) const {
    num_compares_++;
    return a == b;
  }
  bool operator()(const char* a, const char* b) const {
    num_compares_++;
    // The 'a == b' test is necessary, in case a and b are both NULL.
    return (a == b || (a && b && strcmp(a, b) == 0));
  }

 private:
  mutable int id_;
  mutable int num_hashes_;
  mutable int num_compares_;
};

// A transparent hash function that uses the transparent_key_equal to allow
// heterogeneous lookup and keeps track of how often it's called. Similar to
// the hash function above, we use a simple djb-hash so we don't depend on
// how STL hashes.  We use this same method to do the key-comparison, so we
// can keep track of comparison-counts too.
struct TransparentHasher : Hasher {
  using transparent_key_equal = TransparentHasher;
  using is_transparent = void;

  explicit TransparentHasher(int i = 0) : Hasher(i) {}

  using Hasher::operator();

  template <typename T>
  size_t operator()(const pair<T, T>& a) const {
    return this->operator()(a.first);
  }

  template <typename T>
  bool operator()(const pair<T, T>& a, const pair<T, T>& b) const {
    return this->operator()(a.first, b.first);
  }
  template <typename T>
  bool operator()(const T& a, const pair<T, T>& b) const {
    return this->operator()(a, b.first);
  }
  template <typename T>
  bool operator()(const pair<T, T>& a, const T& b) const {
    return this->operator()(a.first, b);
  }
};

// Allocator that allows controlling its size in various ways, to test
// allocator overflow.  Because we use this allocator in a vector, we
// need to define != and swap for gcc.
template <typename T, typename SizeT = size_t,
          SizeT MAX_SIZE = static_cast<SizeT>(~0)>
struct Alloc {
  typedef T value_type;
  typedef SizeT size_type;
  typedef ptrdiff_t difference_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;

  explicit Alloc(int i = 0, int* count = NULL) : id_(i), count_(count) {}
  ~Alloc() {}
  pointer address(reference r) const { return &r; }
  const_pointer address(const_reference r) const { return &r; }
  pointer allocate(size_type n, const_pointer = 0) {
    if (count_) ++(*count_);
    return static_cast<pointer>(malloc(n * sizeof(value_type)));
  }
  void deallocate(pointer p, size_type) { free(p); }
  pointer reallocate(pointer p, size_type n) {
    if (count_) ++(*count_);
    return static_cast<pointer>(realloc(p, n * sizeof(value_type)));
  }
  size_type max_size() const { return static_cast<size_type>(MAX_SIZE); }
  void construct(pointer p, const value_type& val) { new (p) value_type(val); }
  void destroy(pointer p) { p->~value_type(); }

  bool is_custom_alloc() const { return true; }

  template <class U>
  Alloc(const Alloc<U, SizeT, MAX_SIZE>& that)
      : id_(that.id_), count_(that.count_) {}

  template <class U>
  struct rebind {
    typedef Alloc<U, SizeT, MAX_SIZE> other;
  };

  bool operator==(const Alloc<T, SizeT, MAX_SIZE>& that) const {
    return this->id_ == that.id_ && this->count_ == that.count_;
  }
  bool operator!=(const Alloc<T, SizeT, MAX_SIZE>& that) const {
    return !this->operator==(that);
  }

  int id() const { return id_; }

  // I have to make these public so the constructor used for rebinding
  // can see them.  Normally, I'd just make them private and say:
  //   template<typename U, typename U_SizeT, U_SizeT U_MAX_SIZE> friend struct
  //   Alloc;
  // but MSVC 7.1 barfs on that.  So public it is.  But no peeking!
 public:
  int id_;
  int* count_;
};

// Below are a few fun routines that convert a value into a key, used
// for dense_hashtable and sparse_hashtable.  It's our responsibility
// to make sure, when we insert values into these objects, that the
// values match the keys we insert them under.  To allow us to use
// these routines for SetKey as well, we require all these functions
// be their own inverse: f(f(x)) == x.
template <class Value>
struct Negation {
  typedef Value result_type;
  Value operator()(Value& v) { return -v; }
  const Value operator()(const Value& v) const { return -v; }
};

template <class Value>
struct PairNegation {
  typedef Value result_type;
  Value operator()(Value& v) { return Value(-v.first, -v.second); }
  const Value operator()(const Value& v) const { return Value(-v.first, -v.second); }
};

struct Capital {
  typedef string result_type;
  string operator()(string& s) { return string(1, s[0] ^ 32) + s.substr(1); }
  const string operator()(const string& s) const {
    return string(1, s[0] ^ 32) + s.substr(1);
  }
};

struct Identity {  // lame, I know, but an important case to test.
  typedef const char* result_type;
  const char* operator()(const char* s) const { return s; }
};


// This is just to avoid memory leaks -- it's a global pointer to
// all the memory allocated by UniqueObjectHelper.  We'll use it
// to semi-test sparsetable as well. :-)
extern sparsetable<char*> g_unique_charstar_objects;


// This is an object-generator: pass in an index, and it will return a
// unique object of type ItemType.  We provide specializations for the
// types we actually support.
template <typename ItemType>
ItemType UniqueObjectHelper(int index);
template <> int UniqueObjectHelper(int index);
template <> string UniqueObjectHelper(int index);
template <> char* UniqueObjectHelper(int index);
template <> const char* UniqueObjectHelper(int index);
template <> ValueType UniqueObjectHelper(int index);
template <> pair<const int, int> UniqueObjectHelper(int index);
template <> pair<const string, string> UniqueObjectHelper(int index);
template <> pair<const char* const, ValueType> UniqueObjectHelper(int index);
template <> pair<int, int> UniqueObjectHelper(int index);
template <> pair<const pair<int, int>, int> UniqueObjectHelper(int index);

template <typename HashtableType>
class HashtableTest : public ::testing::Test {
 public:
  HashtableTest() : ht_() {}
  // Give syntactically-prettier access to UniqueObjectHelper.
  typename HashtableType::value_type UniqueObject(int index) {
    return UniqueObjectHelper<typename HashtableType::value_type>(index);
  }
  typename HashtableType::key_type UniqueKey(int index) {
    return this->ht_.get_key(this->UniqueObject(index));
  }

 private:
  template<typename T>
  T get_lookup_key(const T& key) {
    printf("get_lookup_key\n");
    return key;
  }

  template<typename T, typename U>
  T get_lookup_key(const pair<T, U>& key) {
    printf("get_lookup_key pair\n");
    return key.first;
  }

 public:
  auto UniqueLookupKey(int index) -> decltype(this->get_lookup_key(this->UniqueKey(index))) {
    return this->get_lookup_key(this->UniqueKey(index));
  }

 protected:
  HashtableType ht_;
};

// These are used to specify the empty key and deleted key in some
// contexts.  They can't be in the unnamed namespace, or static,
// because the template code requires external linkage.
extern const string kEmptyString;
extern const string kDeletedString;
extern const int kEmptyInt;
extern const pair<int, int> kEmptyIntPair;
extern const int kDeletedInt;  // an unlikely-to-pick int
extern const char* const kEmptyCharStar;
extern const char* const kDeletedCharStar;
extern const pair<int, int> kDeletedIntPair;

// Third table has key associated with a value of -value
#define INT_HASHTABLES                                                       \
  HashtableInterface_SparseHashMap<int, int, Hasher, Hasher, Alloc<int>>,    \
      HashtableInterface_SparseHashSet<int, Hasher, Hasher, Alloc<int>>,     \
      HashtableInterface_SparseHashtable<int, int, Hasher, Negation<int>,    \
                                         SetKey<int, Negation<int>>, Hasher, \
                                         Alloc<int>>,                        \
      HashtableInterface_DenseHashMap<int, int, kEmptyInt, Hasher, Hasher,   \
                                      Alloc<int>>,                           \
      HashtableInterface_DenseHashSet<int, kEmptyInt, Hasher, Hasher,        \
                                      Alloc<int>>,                           \
      HashtableInterface_DenseHashtable<                                     \
          int, int, kEmptyInt, Hasher, Negation<int>,                        \
          SetKey<int, Negation<int>>, Hasher, Alloc<int>>

#define TRANSPARENT_INT_HASHTABLES                                            \
  HashtableInterface_SparseHashMap<int, int, TransparentHasher,               \
                                       TransparentHasher, Alloc<int>>,        \
      HashtableInterface_SparseHashSet<int, TransparentHasher,                \
                                       TransparentHasher, Alloc<int>>,        \
      HashtableInterface_SparseHashtable<                                     \
          int, int, TransparentHasher, Negation<int>,                         \
          SetKey<int, Negation<int>>, TransparentHasher, Alloc<int>>,         \
      HashtableInterface_DenseHashMap<int, int, kEmptyInt, TransparentHasher, \
          TransparentHasher, Alloc<int>>,                                     \
      HashtableInterface_DenseHashSet<int, kEmptyInt, TransparentHasher,      \
                                      TransparentHasher, Alloc<int>>,         \
      HashtableInterface_DenseHashtable<                                      \
          int, int, kEmptyInt, TransparentHasher, Negation<int>,              \
          SetKey<int, Negation<int>>, TransparentHasher, Alloc<int>>

// Third table has key associated with a value of Cap(value)
#define STRING_HASHTABLES                                                      \
  HashtableInterface_SparseHashMap<string, string, Hasher, Hasher,             \
                                   Alloc<string>>,                             \
      HashtableInterface_SparseHashSet<string, Hasher, Hasher, Alloc<string>>, \
      HashtableInterface_SparseHashtable<string, string, Hasher, Capital,      \
                                         SetKey<string, Capital>, Hasher,      \
                                         Alloc<string>>,                       \
      HashtableInterface_DenseHashMap<string, string, kEmptyString, Hasher,    \
                                      Hasher, Alloc<string>>,                  \
      HashtableInterface_DenseHashSet<string, kEmptyString, Hasher, Hasher,    \
                                      Alloc<string>>,                          \
      HashtableInterface_DenseHashtable<string, string, kEmptyString, Hasher,  \
                                        Capital, SetKey<string, Capital>,      \
                                        Hasher, Alloc<string>>

// I'd like to use ValueType keys for SparseHashtable<> and
// DenseHashtable<> but I can't due to memory-management woes (nobody
// really owns the char* involved).  So instead I do something simpler.
// Third table has a value equal to its key
#define CHARSTAR_HASHTABLES                                                   \
  HashtableInterface_SparseHashMap<const char*, ValueType, Hasher, Hasher,    \
                                   Alloc<const char*>>,                       \
      HashtableInterface_SparseHashSet<const char*, Hasher, Hasher,           \
                                       Alloc<const char*>>,                   \
      HashtableInterface_SparseHashtable<                                     \
          const char*, const char*, Hasher, Identity,                         \
          SetKey<const char*, Identity>, Hasher, Alloc<const char*>>,         \
      HashtableInterface_DenseHashMap<const char*, ValueType, kEmptyCharStar, \
                                      Hasher, Hasher, Alloc<const char*>>,    \
      HashtableInterface_DenseHashSet<const char*, kEmptyCharStar, Hasher,    \
                                      Hasher, Alloc<const char*>>,            \
      HashtableInterface_DenseHashtable<                                      \
          const char*, const char*, kEmptyCharStar, Hasher, Identity,         \
          SetKey<const char*, Identity>, Hasher, Alloc<ValueType>>

#define INT_PAIR_HASHTABLES                                                 \
  HashtableInterface_SparseHashMap<pair<int, int>, int, TransparentHasher,  \
                                   TransparentHasher, Alloc<int>>,          \
      HashtableInterface_SparseHashSet<pair<int, int>, TransparentHasher,   \
                                       TransparentHasher, Alloc<int>>,      \
      HashtableInterface_SparseHashtable<                                   \
          pair<int, int>, pair<int, int>, TransparentHasher,                \
          PairNegation<pair<int, int>>, SetKey<pair<int, int>,              \
          PairNegation<pair<int, int>>>, TransparentHasher,                 \
          Alloc<int>>,                                                      \
      HashtableInterface_DenseHashMap<                                      \
          pair<int, int>, int, kEmptyIntPair, TransparentHasher,            \
          TransparentHasher, Alloc<int>>,                                   \
      HashtableInterface_DenseHashSet<                                      \
          pair<int, int>, kEmptyIntPair, TransparentHasher,                 \
          TransparentHasher, Alloc<int>>,                                   \
      HashtableInterface_DenseHashtable<                                    \
          pair<int, int>, pair<int, int>, kEmptyIntPair, TransparentHasher, \
          PairNegation<pair<int, int>>, SetKey<pair<int, int>,              \
          PairNegation<pair<int, int>>>, TransparentHasher,                 \
          Alloc<int>>

// This is the list of types we run each test against.
// We need to define the same class 4 times due to limitations in the
// testing framework.  Basically, we associate each class below with
// the set of types we want to run tests on it with.
template <typename HashtableType>
class HashtableIntTest : public HashtableTest<HashtableType> {};
template <typename HashtableType>
class HashtableStringTest : public HashtableTest<HashtableType> {};
template <typename HashtableType>
class HashtableCharStarTest : public HashtableTest<HashtableType> {};
template <typename HashtableType>
class HashtableHeterogeneousLookupTest : public HashtableTest<HashtableType> {};
template <typename HashtableType>
class HashtableAllTest : public HashtableTest<HashtableType> {};

typedef testing::Types<INT_HASHTABLES, TRANSPARENT_INT_HASHTABLES> IntHashtables;
typedef testing::Types<STRING_HASHTABLES> StringHashtables;
typedef testing::Types<CHARSTAR_HASHTABLES> CharStarHashtables;
typedef testing::Types<INT_PAIR_HASHTABLES> IntPairHashtables;
typedef testing::Types<INT_HASHTABLES, TRANSPARENT_INT_HASHTABLES, STRING_HASHTABLES,
                       CHARSTAR_HASHTABLES, INT_PAIR_HASHTABLES> AllHashtables;

TYPED_TEST_CASE(HashtableIntTest, IntHashtables);
TYPED_TEST_CASE(HashtableStringTest, StringHashtables);
TYPED_TEST_CASE(HashtableCharStarTest, CharStarHashtables);
TYPED_TEST_CASE(HashtableHeterogeneousLookupTest, IntPairHashtables);
TYPED_TEST_CASE(HashtableAllTest, AllHashtables);
