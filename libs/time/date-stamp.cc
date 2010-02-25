//==========================================================================
// ObTools::Time: date-stamp.cc
//
// Date stamp functions
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-time.h"
#include <stdlib.h>

namespace ObTools { namespace Time {

//------------------------------------------------------------------------
// Constructor from string
// See ot-time.h for details
DateStamp::DateStamp(const string& text)
{
  string::size_type pos=0;

  // Clear stamp incase we fail
  t=0;

  // Accumulate a Split structure to convert later
  Split split;

  // If length is not at least 8, bomb out now
  string::size_type l = text.size();
  if (l < 8) return;

  // Read year
  string sy(text, pos, 4);
  split.year = atoi(sy.c_str());
  pos+=4;

  // Check for dash
  if (text[pos] == '-') pos++;

  // Read month
  string sm(text, pos, 2);
  split.month = atoi(sm.c_str());
  pos+=2;

  // Check for dash
  if (text[pos] == '-') pos++;

  // Check size - could be close to original 8
  if (pos > l-2) return;

  // Read day
  string sd(text, pos, 2);
  split.day = atoi(sd.c_str());
  pos+=2;

  // Set timestamp
  t = combine(split);
}

}} // namespaces
