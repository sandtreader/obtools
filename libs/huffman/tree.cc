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
// Read an individual character from a BitReader
bool Tree::read_char(Channel::BitReader& reader, unsigned char& c) const
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
        c = node->get_value();
        return true;
      }
    }
    catch (Channel::Error &e)
    {
      return false;
    }
  }
}

}} // namespaces
