//==========================================================================
// ObTools::Huffman: test-reader.cc
//
// Test harness for Huffman Reader classes
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include <fstream>
#include "ot-huffman.h"

using namespace std;
using namespace ObTools;

TEST(TreeReader, TestMultiReader)
{
  ifstream is("../tests/mapping.txt");
  ASSERT_TRUE(is.is_open()) << "Failed to open test mapping file" << endl;
  Huffman::MultiReader mr(is);
  Huffman::MultiMapping mm;

  ASSERT_TRUE(mr.read_mapping(mm));
  ASSERT_EQ(Huffman::Value(Huffman::Value::START), mm.index);
  vector<bool> sequence;
  sequence.push_back(false);
  sequence.push_back(false);
  ASSERT_EQ(sequence.size(), mm.sequence.size());
  for (size_t i = 0; i < mm.sequence.size(); ++i)
    ASSERT_EQ(sequence[i], mm.sequence[i]);
  ASSERT_EQ(Huffman::Value('T'), mm.value);

  ASSERT_TRUE(mr.read_mapping(mm));
  ASSERT_EQ(Huffman::Value('p'), mm.index);
  sequence.clear();
  sequence.push_back(true);
  sequence.push_back(false);
  sequence.push_back(true);
  ASSERT_EQ(sequence.size(), mm.sequence.size());
  for (size_t i = 0; i < mm.sequence.size(); ++i)
    ASSERT_EQ(sequence[i], mm.sequence[i]);
  ASSERT_EQ(Huffman::Value(Huffman::Value::ESCAPE), mm.value);

  ASSERT_TRUE(mr.read_mapping(mm));
  ASSERT_EQ(Huffman::Value('x'), mm.index);
  sequence.clear();
  sequence.push_back(false);
  sequence.push_back(true);
  ASSERT_EQ(sequence.size(), mm.sequence.size());
  for (size_t i = 0; i < mm.sequence.size(); ++i)
    ASSERT_EQ(sequence[i], mm.sequence[i]);
  ASSERT_EQ(Huffman::Value(':'), mm.value);
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
