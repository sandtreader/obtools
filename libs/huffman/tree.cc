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

//==========================================================================
// Tree

//--------------------------------------------------------------------------
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
    catch (const Channel::Error& e)
    {
      return false;
    }
  }
}

//--------------------------------------------------------------------------
// Read an individual value by sequence
bool Tree::read_value(const vector<bool>& sequence, Value& value) const
{
  const Node *node = root.get_node(sequence);
  if (!node)
    return false;

  value = node->get_value();
  return true;
}

//--------------------------------------------------------------------------
// Add a mapping. Will create any necessary nodes for it
bool Tree::add_mapping(const Mapping& m)
{
  Node &node = root.ensure_node(m.sequence);
  node.set_value(m.value);
  return true;
}

//==========================================================================
// MultiTree

//--------------------------------------------------------------------------
// Read an individual value by sequence
bool MultiTree::read_value(const Value& index, const vector<bool>& sequence,
                           Value& value) const
{
  map<Value, Tree>::const_iterator tree = trees.find(index);

  if (tree == trees.end())
    return false;

  return tree->second.read_value(sequence, value);
}

//--------------------------------------------------------------------------
// Read an individual value from a BitReader
bool MultiTree::read_value(const Value& index,
                           Channel::BitReader& reader, Value& value) const
{
  map<Value, Tree>::const_iterator tree = trees.find(index);

  if (tree == trees.end())
    return false;

  return tree->second.read_value(reader, value);
}

//--------------------------------------------------------------------------
// Populate trees from a Huffman reader
bool MultiTree::populate_from(MultiReader& reader)
{
  MultiMapping mm;
  while (reader.read_mapping(mm))
  {
    // Creates new tree if necessary
    Tree& tree = trees[mm.index];

    if (!tree.add_mapping(mm))      // GCOV_EXCL_LINE - add_mapping always succeeds
      return false;                 // GCOV_EXCL_LINE
  }
  return true;
}

//--------------------------------------------------------------------------
// Read an escaped character from a BitReader
bool MultiTree::read_escaped_char(Channel::BitReader& reader,
                                  unsigned char& c) const
{
  try
  {
    c = reader.read_bits(8);
    return true;
  }
  catch (const Channel::Error& e)
  {
    return false;
  }
}

//--------------------------------------------------------------------------
// Read a string from a BitReader
bool MultiTree::read_string(Channel::BitReader& reader, string& s) const
{
  Value value(Huffman::Value::Special::start);
  while (read_value(value, reader, value))
  {
    if (!value.is_special())
      s.push_back(value.get_value());
    else if (value.get_special_value() == Huffman::Value::Special::stop)
      break;
    else if (value.get_special_value() == Huffman::Value::Special::escape)
    {
      // Escaped data is read 8 bits at a time until an ASCII (0-127)
      // character is read, then huffman decoding begins again
      unsigned char c;
      bool more(true);
      while ((more = read_escaped_char(reader, c)) && (c & 0x80))
        s.push_back(c);

      // Check we haven't run out of data
      if (!more)
        return true;

      s.push_back(c);
      value = Value(c);
    }
  }
  return true;
}

}} // namespaces
