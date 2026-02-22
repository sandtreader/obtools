//==========================================================================
// ObTools::Huffman: test-value-format.cc
//
// Test harness for Huffman value operator<< and special values
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-huffman.h"
#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace std;
using namespace ObTools;

TEST(ValueFormatTest, TestOutputRegularValue)
{
  Huffman::Value v('A');
  ostringstream oss;
  oss << v;
  EXPECT_EQ("A", oss.str());
}

TEST(ValueFormatTest, TestOutputSpecialStart)
{
  Huffman::Value v(Huffman::Value::Special::start);
  ostringstream oss;
  oss << v;
  EXPECT_EQ("START", oss.str());
}

TEST(ValueFormatTest, TestOutputSpecialStop)
{
  Huffman::Value v(Huffman::Value::Special::stop);
  ostringstream oss;
  oss << v;
  EXPECT_EQ("STOP", oss.str());
}

TEST(ValueFormatTest, TestOutputSpecialEscape)
{
  Huffman::Value v(Huffman::Value::Special::escape);
  ostringstream oss;
  oss << v;
  EXPECT_EQ("ESCAPE", oss.str());
}

TEST(ValueFormatTest, TestOutputSpecialNone)
{
  Huffman::Value v(Huffman::Value::Special::none);
  ostringstream oss;
  oss << v;
  EXPECT_EQ("NONE", oss.str());
}

TEST(ValueFormatTest, TestDefaultValueIsInvalid)
{
  Huffman::Value v;
  EXPECT_TRUE(!v);
}

TEST(ValueFormatTest, TestRegularValueIsValid)
{
  Huffman::Value v('X');
  EXPECT_FALSE(!v);
}

TEST(ValueFormatTest, TestSpecialValueIsSpecial)
{
  Huffman::Value v(Huffman::Value::Special::start);
  EXPECT_TRUE(v.is_special());
  EXPECT_EQ(Huffman::Value::Special::start, v.get_special_value());
}

TEST(ValueFormatTest, TestRegularValueNotSpecial)
{
  Huffman::Value v('A');
  EXPECT_FALSE(v.is_special());
  EXPECT_EQ('A', v.get_value());
}

TEST(ValueFormatTest, TestValueEquality)
{
  Huffman::Value v1('A');
  Huffman::Value v2('A');
  Huffman::Value v3('B');
  EXPECT_TRUE(v1 == v2);
  EXPECT_FALSE(v1 == v3);
}

TEST(ValueFormatTest, TestValueLessThan)
{
  // Regular < regular
  Huffman::Value v1('A');
  Huffman::Value v2('B');
  EXPECT_TRUE(v1 < v2);
  EXPECT_FALSE(v2 < v1);

  // Regular < special (regular should be less than special)
  Huffman::Value vs(Huffman::Value::Special::start);
  EXPECT_TRUE(v1 < vs);
  EXPECT_FALSE(vs < v1);

  // Special < special (by enum value)
  Huffman::Value vs2(Huffman::Value::Special::stop);
  EXPECT_TRUE(vs < vs2);
}

//--------------------------------------------------------------------------
// Node::get_node(sequence) where child doesn't exist
TEST(ValueFormatTest, TestNodeGetNodeMissingChild)
{
  Huffman::Node root;
  Huffman::Node leaf('A');
  root.set_node(false, leaf); // only 0-branch exists

  vector<bool> seq = {true, false}; // 1-branch doesn't exist
  EXPECT_EQ(nullptr, root.get_node(seq));
}

//--------------------------------------------------------------------------
// Tree::read_value(sequence) when node not found
TEST(ValueFormatTest, TestTreeReadValueSequenceNotFound)
{
  Huffman::Tree tree;
  Huffman::Mapping m;
  m.sequence = {false, false};
  m.value = Huffman::Value('A');
  tree.add_mapping(m);

  // Query a sequence that doesn't exist
  vector<bool> bad_seq = {true, true, true};
  Huffman::Value val;
  EXPECT_FALSE(tree.read_value(bad_seq, val));
}

//--------------------------------------------------------------------------
// Tree::read_value(BitReader) when data has invalid path
TEST(ValueFormatTest, TestTreeReadValueBitReaderInvalidPath)
{
  Huffman::Tree tree;
  Huffman::Mapping m;
  m.sequence = {false};
  m.value = Huffman::Value('A');
  tree.add_mapping(m);

  // Feed bit 1 which has no mapping
  string data;
  data += (char)0x80; // 1-bit first
  Channel::StringReader sr(data);
  Channel::BitReader br(sr);
  Huffman::Value val;
  EXPECT_FALSE(tree.read_value(br, val));
}

//--------------------------------------------------------------------------
// MultiTree::read_value(index, sequence) when tree doesn't exist
TEST(ValueFormatTest, TestMultiTreeReadValueMissingTree)
{
  Huffman::MultiTree mt;
  // Don't populate any trees
  vector<bool> seq = {false};
  Huffman::Value val;
  EXPECT_FALSE(mt.read_value(Huffman::Value('X'), seq, val));
}

//--------------------------------------------------------------------------
// MultiTree::read_value(index, BitReader) when tree doesn't exist
TEST(ValueFormatTest, TestMultiTreeReadValueBitReaderMissingTree)
{
  Huffman::MultiTree mt;
  string data;
  data += (char)0x80;
  Channel::StringReader sr(data);
  Channel::BitReader br(sr);
  Huffman::Value val;
  EXPECT_FALSE(mt.read_value(Huffman::Value('X'), br, val));
}

//--------------------------------------------------------------------------
// MultiReader: blank lines skipped, invalid lines return false
TEST(ValueFormatTest, TestMultiReaderBlankLinesSkipped)
{
  // Blank line followed by valid mapping
  istringstream iss("\na:01:b:\n");
  Huffman::MultiReader mr(iss);
  Huffman::MultiMapping mm;
  EXPECT_TRUE(mr.read_mapping(mm));
  EXPECT_EQ(Huffman::Value('a'), mm.index);
  EXPECT_EQ(Huffman::Value('b'), mm.value);
}

TEST(ValueFormatTest, TestMultiReaderWrongPartCount)
{
  // Line with only 2 colon-separated parts (needs 3)
  istringstream iss("a:01\n");
  Huffman::MultiReader mr(iss);
  Huffman::MultiMapping mm;
  EXPECT_FALSE(mr.read_mapping(mm));
}

TEST(ValueFormatTest, TestMultiReaderEmptyIndex)
{
  // Empty index value
  istringstream iss(":01:b:\n");
  Huffman::MultiReader mr(iss);
  Huffman::MultiMapping mm;
  EXPECT_FALSE(mr.read_mapping(mm));
}

TEST(ValueFormatTest, TestMultiReaderEmptyValue)
{
  // Empty target value
  istringstream iss("a:01::\n");
  Huffman::MultiReader mr(iss);
  Huffman::MultiMapping mm;
  EXPECT_FALSE(mr.read_mapping(mm));
}

//--------------------------------------------------------------------------
// MultiTree::read_string with truncated escape data
TEST(ValueFormatTest, TestMultiTreeReadStringTruncatedEscape)
{
  // Build a tree with an escape path
  const char *mapping = "START:1:ESCAPE:\n";
  istringstream iss(mapping);
  Huffman::MultiReader mr(iss);
  Huffman::MultiTree tree;
  tree.populate_from(mr);

  // Feed just the escape bit (1) then truncate - no escaped chars
  vector<unsigned char> data;
  data.push_back(0x80); // 1 = ESCAPE, then no more valid data for 8-bit read
  Channel::BlockReader blr(&data[0], data.size());
  Channel::BitReader br(blr);
  string s;
  // Should return true (graceful handling of truncated escape)
  EXPECT_TRUE(tree.read_string(br, s));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
