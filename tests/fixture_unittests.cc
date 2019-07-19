#include "fixture_unittests.h"


// This is just to avoid memory leaks -- it's a global pointer to
// all the memory allocated by UniqueObjectHelper.  We'll use it
// to semi-test sparsetable as well. :-)
sparsetable<char*> g_unique_charstar_objects = sparsetable<char*>(16);

// These are used to specify the empty key and deleted key in some
// contexts.  They can't be in the unnamed namespace, or static,
// because the template code requires external linkage.
const string kEmptyString("--empty string--");
const string kDeletedString("--deleted string--");
const int kEmptyInt = 0;
const int kDeletedInt = -1234676543;  // an unlikely-to-pick int
const char* const kEmptyCharStar = "--empty char*--";
const char* const kDeletedCharStar = "--deleted char*--";
const pair<int, int> kEmptyIntPair = {0, 0};
const pair<int, int> kDeletedIntPair = {-1234676543, -1234676543};

const char* const ValueType::kDefault = "hi";

template <>
int UniqueObjectHelper(int index) {
  return index;
}

template <>
string UniqueObjectHelper(int index) {
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "%d", index);
  return buffer;
}

template <>
char* UniqueObjectHelper(int index) {
  // First grow the table if need be.
  sparsetable<char*>::size_type table_size = g_unique_charstar_objects.size();
  while (index >= static_cast<int>(table_size)) {
    assert(table_size * 2 > table_size);  // avoid overflow problems
    table_size *= 2;
  }
  if (table_size > g_unique_charstar_objects.size())
    g_unique_charstar_objects.resize(table_size);

  if (!g_unique_charstar_objects.test(index)) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%d", index);
    g_unique_charstar_objects[index] = strdup(buffer);
  }
  return g_unique_charstar_objects.get(index);
}

template <>
const char* UniqueObjectHelper(int index) {
  return UniqueObjectHelper<char*>(index);
}

template <>
ValueType UniqueObjectHelper(int index) {
  return ValueType(UniqueObjectHelper<string>(index).c_str());
}

template <>
pair<const int, int> UniqueObjectHelper(int index) {
  return pair<const int, int>(index, index + 1);
}

template <>
pair<const string, string> UniqueObjectHelper(int index) {
  return pair<const string, string>(UniqueObjectHelper<string>(index),
                                    UniqueObjectHelper<string>(index + 1));
}

template <>
pair<const char* const, ValueType> UniqueObjectHelper(int index) {
  return pair<const char* const, ValueType>(
      UniqueObjectHelper<char*>(index),
      UniqueObjectHelper<ValueType>(index + 1));
}

template <>
pair<int, int> UniqueObjectHelper(int index) {
  return pair<int, int>(index, index + 1);
}

template <>
pair<const pair<int, int>, int> UniqueObjectHelper(int index) {
  return pair<const pair<int, int>, int>(
      UniqueObjectHelper<pair<int, int>>(index),
      index + 2);
}
