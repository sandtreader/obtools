//==========================================================================
// ObTools::Huffman: node.cc
//
// Huffman Node implementation
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-huffman.h"

namespace ObTools { namespace Huffman {

//========================================================================
// Get the node for a bit sequence
// returns null if no node found
const Node *Node::get_node(vector<bool> sequence) const
{
  if (sequence.empty())
    return this;

  // It's not this node so pass it along a place
  map<bool, Node>::const_iterator next = nodes.find(sequence[0]);
  sequence.erase(sequence.begin());
  return next->second.get_node(sequence);
}

//========================================================================
// Ensures a node for the given sequence exists and returns a reference
// to it
Node& Node::ensure_node(vector<bool> sequence)
{
  if (sequence.empty())
    return *this;

  // It's not this node so pass it along a place
  Node& node = nodes[sequence[0]];
  sequence.erase(sequence.begin());
  return node.ensure_node(sequence);
}

}} // namespaces
