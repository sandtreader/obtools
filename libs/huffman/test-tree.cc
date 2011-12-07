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

using namespace std;
using namespace ObTools;

TEST(TreeTest, TestLeafyNode)
{
  Huffman::Node node('a');
  ASSERT_TRUE(node.is_leaf());
  ASSERT_EQ('a', node.get_value());
}

TEST(TreeTest, TestForkingNode)
{
  Huffman::Node node1;
  Huffman::Node node2('a');
  Huffman::Node node3('b');
  node1.set_node(false, node2);
  node1.set_node(true, node3);
  ASSERT_FALSE(node1.is_leaf());
  ASSERT_EQ('a', node1.get_node(false)->get_value());
  ASSERT_EQ('b', node1.get_node(true)->get_value());
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

  unsigned char a;
  unsigned char b;
  unsigned char c;
  ASSERT_TRUE(tree.read_char(br, a));
  ASSERT_TRUE(tree.read_char(br, b));
  ASSERT_TRUE(tree.read_char(br, c));
  ASSERT_EQ('a', a);
  ASSERT_EQ('b', b);
  ASSERT_EQ('c', c);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
