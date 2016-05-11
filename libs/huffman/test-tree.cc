//==========================================================================
// ObTools::Huffman: test-tree.cc
//
// Test harness for Huffman Tree class
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-huffman.h"
#include <vector>
#include <fstream>

using namespace std;
using namespace ObTools;

TEST(TreeTest, TestLeafyNode)
{
  Huffman::Node node('a');
  ASSERT_TRUE(node.is_leaf());
  ASSERT_EQ(Huffman::Value('a'), node.get_value());
}

TEST(TreeTest, TestForkingNode)
{
  Huffman::Node node1;
  Huffman::Node node2('a');
  Huffman::Node node3('b');
  node1.set_node(false, node2);
  node1.set_node(true, node3);
  ASSERT_FALSE(node1.is_leaf());
  ASSERT_EQ(Huffman::Value('a'), node1.get_node(false)->get_value());
  ASSERT_EQ(Huffman::Value('b'), node1.get_node(true)->get_value());
}

TEST(TreeTest, TestGetChar)
{
  Huffman::Tree tree;
  Huffman::Node node1;
  Huffman::Node node2('a');
  Huffman::Node node3('b');
  Huffman::Node node4('c');
  node1.set_node(false, node2);
  node1.set_node(true, node3);
  tree.set_node(false, node1);
  tree.set_node(true, node4);
  vector<unsigned char> data(8);
  Channel::BlockWriter blw(&data[0], data.size());
  Channel::BitWriter bw(blw);
  // abc
  bw.write_bit(0);
  bw.write_bit(0);

  bw.write_bit(0);
  bw.write_bit(1);

  bw.write_bit(1);

  // Fill out to a byte
  bw.write_bit(0);
  bw.write_bit(0);
  bw.write_bit(0);

  Channel::BlockReader blr(&data[0], data.size());
  Channel::BitReader br(blr);

  Huffman::Value a;
  Huffman::Value b;
  Huffman::Value c;
  ASSERT_TRUE(tree.read_value(br, a));
  ASSERT_TRUE(tree.read_value(br, b));
  ASSERT_TRUE(tree.read_value(br, c));
  ASSERT_EQ(Huffman::Value('a'), a);
  ASSERT_EQ(Huffman::Value('b'), b);
  ASSERT_EQ(Huffman::Value('c'), c);
}

TEST(TestMultiTree, TestPopulateFromReader)
{
  ifstream ifs("../tests/mapping.txt");
  ASSERT_TRUE(ifs.is_open()) << "Failed to open test mapping file" << endl;
  Huffman::MultiReader mr(ifs);
  Huffman::MultiTree tree;
  tree.populate_from(mr);
  vector<bool> sequence;
  sequence.push_back(true);
  sequence.push_back(false);
  sequence.push_back(true);
  Huffman::Value value;
  ASSERT_TRUE(tree.read_value(Huffman::Value('p'), sequence, value));
  ASSERT_EQ(Huffman::Value(Huffman::Value::Special::escape), value);
}

TEST(TestMultiTree, TestReadString)
{
  // Set up tree
  ifstream ifs("../tests/mapping.txt");
  ASSERT_TRUE(ifs.is_open()) << "Failed to open test mapping file" << endl;
  Huffman::MultiReader mr(ifs);
  Huffman::MultiTree tree;
  tree.populate_from(mr);

  // Testing begins
  vector<unsigned char> data;
  data.push_back(0x5A); // 01-a 0110-b 10..
  data.push_back(0xC0); // 11-c 00-STOP 0000
  Channel::BlockReader blr(&data[0], data.size());
  Channel::BitReader br(blr);
  string s;
  ASSERT_TRUE(tree.read_string(br, s));
  ASSERT_EQ("abc", s);
}

TEST(TestMultiTree, TestReadStringWithEscaping)
{
  // Set up tree
  ifstream ifs("../tests/mapping.txt");
  ASSERT_TRUE(ifs.is_open()) << "Failed to open test mapping file" << endl;
  Huffman::MultiReader mr(ifs);
  Huffman::MultiTree tree;
  tree.populate_from(mr);

  // Testing begins
  vector<unsigned char> data;
  data.push_back(0x5A); // 01-a 0110-b 10..
  data.push_back(0xDE); // 11-c 01-d 1-ESCAPE 110..
  data.push_back(0x1D); // 00011-101...
  data.push_back(0x4B); // 01001-é(UTF-8) 011..
  data.push_back(0x33); // 00110-f 011-g
  data.push_back(0x00); // 0-STOP 0000000
  Channel::BlockReader blr(&data[0], data.size());
  Channel::BitReader br(blr);
  string s;
  ASSERT_TRUE(tree.read_string(br, s));
  ASSERT_EQ("abcdéfg", s);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
