//==========================================================================
// ObTools::Tube: tag.cc
//
// Implementation of tag to string conversions
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-tube.h"
#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace Tube {

//------------------------------------------------------------------------
// Get a friendly string version of a tag
string tag_to_string(tag_t tag)
{
  string s;
  for(int i=0; i<4; i++)
  {
    char c = static_cast<char>(tag >> (24-8*i));  // Big-endian
    if (isprint(c)) 
      s+=c;
    else 
      goto non_print;
  }

  return s;

non_print:
  // Land here if any character is non-printable - just give hex verson
  return Text::itox(tag);
}

//------------------------------------------------------------------------
// Get a tag from a string
tag_t string_to_tag(const string& str)
{
  tag_t tag = 0;
  for(unsigned int i=0; i<4 && i<str.size(); i++)
  {
    tag = (tag << 8) | static_cast<int>(str[i]);
  }

  return tag;
}

}} // namespaces




