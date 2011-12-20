//==========================================================================
// ObTools::Huffman: tree.cc
//
// Huffman Tree implementation
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-huffman.h"

namespace ObTools { namespace Huffman {

//========================================================================
// Tree

//------------------------------------------------------------------------
// Read an individual character from a BitReader
bool Tree::read_value(Channel::BitReader& reader, Value& value) const
{
  const Node *node = &root;
  while (true)
  {
    try
    {
      bool bit = reader.read_bool();
      node = node->get_node(bit);
      if (!node)
        return false;
      if (node->is_leaf())
      {
        value = node->get_value();
        return true;
      }
    }
    catch (Channel::Error &e)
    {
      return false;
    }
  }
}

//------------------------------------------------------------------------
// Read an individual value by sequence
bool Tree::read_value(const vector<bool>& sequence, Value& value) const
{
  const Node *node = root.get_node(sequence);
  if (!node)
    return false;

  value = node->get_value();
  return true;
}

//------------------------------------------------------------------------
// Add a mapping. Will create any necessary nodes for it
bool Tree::add_mapping(const Mapping& m)
{
  Node &node = root.ensure_node(m.sequence);
  node.set_value(m.value);
  return true;
}

//========================================================================
// MultiTree

//------------------------------------------------------------------------
// Read an individual value by sequence
bool MultiTree::read_value(const Value& index, const vector<bool>& sequence,
                           Value& value) const
{
  map<Value, Tree>::const_iterator tree = trees.find(index);

  if (tree == trees.end())
    return false;

  return tree->second.read_value(sequence, value);
}

//------------------------------------------------------------------------
// Read an individual value from a BitReader
bool MultiTree::read_value(const Value& index,
                           Channel::BitReader& reader, Value& value) const
{
  map<Value, Tree>::const_iterator tree = trees.find(index);

  if (tree == trees.end())
    return false;

  return tree->second.read_value(reader, value);
}

//------------------------------------------------------------------------
// Populate trees from a Huffman reader
bool MultiTree::populate_from(MultiReader& reader)
{
  MultiMapping mm;
  while (reader.read_mapping(mm))
  {
    // Creates new tree if necessary
    Tree& tree = trees[mm.index];

    if (!tree.add_mapping(mm))
      return false;
  }
  return true;
}

//------------------------------------------------------------------------
// Read a string from a BitReader
bool MultiTree::read_string(Channel::BitReader& reader, string& s) const
{
  Value value(Huffman::Value::START);
  while (read_value(value, reader, value))
  {
    if (!value.is_special())
      s.push_back(value.get_value());
    else if (value.get_special_value() == Huffman::Value::STOP)
      break;
  }
  return true;
}

}} // namespaces
