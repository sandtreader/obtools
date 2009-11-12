//==========================================================================
// ObTools::Tube: message.cc
//
// Implementation of tube messages
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-tube.h"
#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace Tube {

//------------------------------------------------------------------------
// Get a friendly string version of the tag
string Message::stag()
{
  string s = "'";
  for(int i=0; i<4; i++)
  {
    char c = (char)(tag >> (24-8*i));  // Big-endian
    if (isprint(c)) 
      s+=c;
    else 
      goto non_print;
  }

  s+="'";
  return s;

non_print:
  // Land here if any character is non-printable - just give hex verson
  return Text::itox(tag);
}


}} // namespaces




