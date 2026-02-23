//==========================================================================
// ObTools::Huffman: reader.cc
//
// Huffman Reader implementation
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-huffman.h"
#include "ot-text.h"

namespace ObTools { namespace Huffman {

namespace
{
  Value get_value_from_string(const string& str)
  {
    if (str.empty())
      return Value();

    if (str == "START")
      return Value(Value::Special::start);
    else if (str == "STOP")
      return Value(Value::Special::stop);
    else if (str == "ESCAPE")
      return Value(Value::Special::escape);
    else if (str.size() == 4 && str.substr(0, 2) == "0x")
      return Value(static_cast<unsigned char>(Text::xtoi(str.substr(2, 2))));
    else
      return Value(str[0]);
  }

  void get_sequence_from_string(const string& str, vector<bool>& sequence)
  {
    sequence.clear();
    for (string::const_iterator it = str.begin(); it != str.end(); ++it)
      sequence.push_back(*it != '0');
  }
}

bool MultiReader::read_mapping(MultiMapping& mapping)
{
  string line;

  bool got_line(false);

  while (!got_line)
  {
    line.resize(256);
    sin.getline(&line[0], line.size());

    if (sin.eof() || sin.fail())
      return false;

    if (sin.gcount() < 1) // GCOV_EXCL_LINE - getline sets eof/fail first
      continue;           // GCOV_EXCL_LINE

    line.resize(sin.gcount() - 1);

    // Skip blank lines
    if (line.empty())
      continue;

    got_line = true;
  }

  if (line[line.size() - 1] == ':')
    line.resize(line.size() - 1);

  vector<string> bits = Text::split(line, ':', false, 3);
  if (bits.size() != 3)
    return false;

  mapping.index = get_value_from_string(bits[0]);
  if (!mapping.index)
    return false;

  get_sequence_from_string(bits[1], mapping.sequence);

  mapping.value = get_value_from_string(bits[2]);
  if (!mapping.value)
    return false;

  return true;
}

}} // namespaces
