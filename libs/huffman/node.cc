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

//--------------------------------------------------------------------------
// Set the node for a bit value
void Node::set_node(bool bit, const Node& node)
{
  if (bit)
    one.reset(new Node(node));
  else
    zero.reset(new Node(node));
  leaf = false;
}

//--------------------------------------------------------------------------
// Get the node for a bit sequence
// returns null if no node found
const Node *Node::get_node(const vector<bool>& sequence) const
{
  vector<bool>::const_iterator pos = sequence.begin();
  return get_node(sequence, pos);
}

const Node *Node::get_node(const vector<bool>& sequence,
                           vector<bool>::const_iterator& pos) const
{
  if (pos == sequence.end())
    return this;

  // It's not this node so pass it along a place
  Gen::SharedPointer<Node> next;
  if (*pos)
    next = one;
  else
    next = zero;
  ++pos;
  if (next.get())
    return next->get_node(sequence, pos);
  return 0;
}

//--------------------------------------------------------------------------
// Ensures a node for the given sequence exists and returns a reference
// to it
Node& Node::ensure_node(const vector<bool>& sequence)
{
  vector<bool>::const_iterator pos = sequence.begin();
  return ensure_node(sequence, pos);
}

Node& Node::ensure_node(const vector<bool>& sequence,
                        vector<bool>::const_iterator& pos)
{
  if (pos == sequence.end())
    return *this;

  // It's not this node so pass it along a place
  Gen::SharedPointer<Node>& next(*pos ? one : zero);
  ++pos;
  if (!next.get())
    next.reset(new Node());
  return next->ensure_node(sequence, pos);
}

}} // namespaces
