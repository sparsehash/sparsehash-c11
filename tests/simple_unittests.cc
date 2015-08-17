// Copyright (c) 2007, Google Inc.
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

// ---
//
// This tests mostly that we can #include the files correctly
// and have them work. It's meant to emulate what a 'regular
// install' of sparsehash would be able to see.

#include "gtest/gtest.h"
#include <sparsehash/sparse_hash_set>
#include <sparsehash/sparse_hash_map>
#include <sparsehash/dense_hash_set>
#include <sparsehash/dense_hash_map>

using namespace testing;

TEST(Simple, All) {
  google::sparse_hash_set<int> sset;
  google::sparse_hash_map<int, int> smap;
  google::dense_hash_set<int> dset;
  google::dense_hash_map<int, int> dmap;
  dset.set_empty_key(-1);
  dmap.set_empty_key(-1);

  for (int i = 0; i < 100; i += 10) {  // go by tens
    sset.insert(i);
    smap[i] = i + 1;
    dset.insert(i + 5);
    dmap[i + 5] = i + 6;
  }

  for (int i = 0; i < 100; i++) {
    ASSERT_EQ(sset.find(i) != sset.end(), (i % 10) == 0);
    ASSERT_EQ(smap.find(i) != smap.end(), (i % 10) == 0);
    ASSERT_EQ(smap.find(i) != smap.end() && smap.find(i)->second == i + 1,
              (i % 10) == 0);
    ASSERT_EQ(dset.find(i) != dset.end(), (i % 10) == 5);
    ASSERT_EQ(dmap.find(i) != dmap.end(), (i % 10) == 5);
    ASSERT_EQ(dmap.find(i) != dmap.end() && dmap.find(i)->second == i + 1,
              (i % 10) == 5);
  }
}
