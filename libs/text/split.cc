//==========================================================================
// ObTools::Text: split.cc
//
// Useful text splitting functions
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace Text {

//--------------------------------------------------------------------------
// Split a string into fields using the given delimiter
// Canonicalises fields (removed leading and trailing whitespace, folds
// multiple internal whitespace into one) if canonicalise is set
vector<string> split(const string& text, char delim, bool canonicalise)
{
  string::size_type p = 0;
  vector<string> fields;

  for(;;)
  {
    // Find first delimiter
    string::size_type dp = text.find(delim, p);
    if (dp != string::npos)
    {
      // Capture up to here as the field
      string field(text, p, dp-p);
      if (canonicalise) field = canonicalise_space(field);
      fields.push_back(field);
      p = dp+1;
    }
    else
    {
      // Rest of the string
      string field(text, p);
      if (canonicalise) field = canonicalise_space(field);
      fields.push_back(field);
      break;
    }
  }

  return fields;
}

}} // namespaces
