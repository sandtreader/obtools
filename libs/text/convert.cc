//==========================================================================
// ObTools::Text: convert.cc
//
// Useful numeric - string conversion functions
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-text.h"
#include <ctype.h>
#include <sstream>
#include <stdio.h>

namespace ObTools { namespace Text {

//--------------------------------------------------------------------------
// Integer to string
string itos(int i)
{
  // Use sprintf for speed
  char buf[21];  // Enough for 64-bit integers, just in case
  snprintf(buf, 21, "%d", i);
  return string(buf);
}

//--------------------------------------------------------------------------
// String to integer (0 default)
int stoi(const string& s)
{
  return atoi(s.c_str());
}

//--------------------------------------------------------------------------
// Float to string 
string ftos(double f)
{
  // Use sprintf for speed
  char buf[21];  // Enough for %g format
  snprintf(buf, 21, "%.10g", f);
  return string(buf);
}

//--------------------------------------------------------------------------
// String to float (0.0 default)
double stof(const string& s)
{
  return atof(s.c_str());
}

}} // namespaces
