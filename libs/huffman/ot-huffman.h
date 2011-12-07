//==========================================================================
// ObTools::Huffman: ot-huffman.h
//
// Public definitions for ObTools::Huffman
// Huffman coding C++ library:
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_HUFFMAN_H
#define __OBTOOLS_HUFFMAN_H

#include <map>
#include "ot-chan.h"

namespace ObTools { namespace Huffman {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Tree Node class
// Can be a leafy or forking
class Node
{
private:
  bool leaf;
  unsigned char value;
  map<bool, Node> nodes;

public:
  //========================================================================
  // Constructors
  Node():
    leaf(false), value(0)
  {}

  Node(unsigned char _value):
    leaf(true), value(_value)
  {}

  //========================================================================
  // Get value of the node
  unsigned char get_value() const
  {
    return value;
  }

  //========================================================================
  // Is this a leaf node?
  bool is_leaf() const
  {
    return leaf;
  }

  //========================================================================
  // Set the node for a bit value
  void set_node(bool bit, const Node& node)
  {
    nodes[bit] = node;
  }

  //========================================================================
  // Get the node for a bit value
  // returns null if no node found
  const Node *get_node(bool bit) const
  {
    map<bool, Node>::const_iterator it = nodes.find(bit);
    if (it != nodes.end())
      return &it->second;
    return 0;
  }
};

//==========================================================================
// Tree class
class Tree
{
private:
  Node root;

public:
  //========================================================================
  // Set the node for a bit value
  void set_node(bool bit, const Node& node)
  {
    root.set_node(bit, node);
  }

  //========================================================================
  // Get the node for a bit value
  // returns null if no node found
  const Node *get_node(bool bit) const
  {
    return root.get_node(bit);
  }

  //========================================================================
  // Read an individual character from a BitReader
  bool read_char(Channel::BitReader& reader, unsigned char& c) const;
};

//==========================================================================
}} // namespaces
#endif // !__OBTOOLS_HUFFMAN_H
