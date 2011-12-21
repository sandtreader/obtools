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
// A value
class Value
{
public:
  enum Special
  {
    NONE,
    START,
    STOP,
    ESCAPE,
  };
private:
  unsigned char value;
  Special svalue;
  bool special;
  bool valid;

public:
  Value():
    value(0), svalue(NONE), special(false), valid(false)
  {}

  Value(unsigned char _value):
    value(_value), svalue(NONE), special(false), valid(true)
  {}

  Value(Special _svalue):
    value(0), svalue(_svalue), special(true), valid(true)
  {}

  unsigned char get_value() const
  {
    return value;
  }

  Special get_special_value() const
  {
    return svalue;
  }

  bool is_special() const
  {
    return special;
  }

  bool operator!() const
  {
    return !valid;
  }

  bool operator==(const Value& b) const
  {
    return special == b.special && value == b.value;
  }

  bool operator<(const Value &b) const
  {
    if (special)
    {
      if (b.is_special())
        return svalue < b.svalue;
      return false;
    }
    else
    {
      if (b.is_special())
        return true;
      return value < b.value;
    }
  }
};

ostream& operator<<(ostream& os, const Value& value);

//==========================================================================
// Tree Node class
// Can be a leafy or forking
class Node
{
private:
  bool leaf;
  Value value;
  map<bool, Node> nodes;

public:
  //------------------------------------------------------------------------
  // Constructors
  Node():
    leaf(false), value(0)
  {}

  Node(const Value& _value):
    leaf(true), value(_value)
  {}

  //------------------------------------------------------------------------
  // Get value of the node
  const Value& get_value() const
  {
    return value;
  }

  //------------------------------------------------------------------------
  // Set value of the node
  void set_value(const Value& _value)
  {
    value = _value;
    leaf = true;
  }

  //------------------------------------------------------------------------
  // Is this a leaf node?
  bool is_leaf() const
  {
    return leaf;
  }

  //------------------------------------------------------------------------
  // Set the node for a bit value
  void set_node(bool bit, const Node& node)
  {
    nodes[bit] = node;
    leaf = false;
  }

  //------------------------------------------------------------------------
  // Get the node for a bit value
  // returns null if no node found
  const Node *get_node(bool bit) const
  {
    map<bool, Node>::const_iterator it = nodes.find(bit);
    if (it != nodes.end())
      return &it->second;
    return 0;
  }

  //------------------------------------------------------------------------
  // Get the node for a bit sequence
  // returns null if no node found
  const Node *get_node(vector<bool> sequence) const;

  //------------------------------------------------------------------------
  // Ensures a node for the given sequence exists and returns a reference
  // to it
  Node& ensure_node(vector<bool> sequence);
};

//==========================================================================
// Mapping
// represents a mapping from a sequence of bits to a value
class Mapping
{
public:
  vector<bool> sequence;
  Value value;
};

//==========================================================================
// Multi Mapping
// represents a mapping from a sequence of bits to a value for a particular
// previous value
class MultiMapping: public Mapping
{
public:
  Value index;
};

//==========================================================================
// Tree class
class Tree
{
private:
  Node root;

public:
  //------------------------------------------------------------------------
  // Set the node for a bit value
  void set_node(bool bit, const Node& node)
  {
    root.set_node(bit, node);
  }

  //------------------------------------------------------------------------
  // Get the node for a bit value
  // returns null if no node found
  const Node *get_node(bool bit) const
  {
    return root.get_node(bit);
  }

  //------------------------------------------------------------------------
  // Read an individual value from a BitReader
  bool read_value(Channel::BitReader& reader, Value& value) const;

  //------------------------------------------------------------------------
  // Read an individual value by sequence
  bool read_value(const vector<bool>& sequence, Value& value) const;

  //------------------------------------------------------------------------
  // Add a mapping. Will create any necessary nodes for it
  bool add_mapping(const Mapping& m);
};

//==========================================================================
// Multi Reader
// Use for populating a MultiTree from a file
//
// Format of a mapping file is as follows:
// Any line starting with a # will be ignored, as will blank lines
// Every other line should contain a mapping consisting of 3 parts:
// - previous value read
// - a bit sequence
// - a value
// These parts should be separated by colons (:) with an optional end of
// line colon.
// e.g.:
// b:00110:c:
// Values are usually ASCII characters, but some special characters exist:
// START - used for previous value where no data has yet been read
// STOP - indicates end of data
// ESCAPE - indicates escaping from Huffman encoding
// Values can also be specified in 0x hex format, e.g. 0x3a (primarily used
// for specifying colons in the mapping)
class MultiReader
{
private:
  istream& sin;

public:
  MultiReader(istream& s):
    sin(s)
  {}

  bool read_mapping(MultiMapping& mapping);
};

//==========================================================================
// Multi-Tree class
// Holds multiple Huffman trees which are chosen based on the output of the
// previous decoding
class MultiTree
{
private:
  map<Value, Tree> trees;

  //------------------------------------------------------------------------
  // Read an escaped character from a BitReader
  bool read_escaped_char(Channel::BitReader& reader, unsigned char& c) const;

public:

  //------------------------------------------------------------------------
  // Populate trees from a Huffman reader
  bool populate_from(MultiReader& reader);

  //------------------------------------------------------------------------
  // Read an individual value by sequence
  bool read_value(const Value& index, const vector<bool>& sequence,
                  Value& value) const;

  //------------------------------------------------------------------------
  // Read an individual value from a BitReader
  bool read_value(const Value& index,
                  Channel::BitReader& reader, Value& value) const;

  //------------------------------------------------------------------------
  // Read a string from a BitReader
  bool read_string(Channel::BitReader& reader, string& s) const;
};

//==========================================================================
}} // namespaces
#endif // !__OBTOOLS_HUFFMAN_H
