//==========================================================================
// ObTools::Text: case.cc
//
// Useful text case handling functions not provided by standard library
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace Text {

//--------------------------------------------------------------------------
// Lower-case a string
string tolower(const string& text)
{
  string res;
  for(string::const_iterator p = text.begin(); p!=text.end(); p++)
    res+=::tolower(*p);

  return res;
}

//--------------------------------------------------------------------------
// Upper-case a string
string toupper(const string& text)
{
  string res;
  for(string::const_iterator p = text.begin(); p!=text.end(); p++)
    res+=::toupper(*p);

  return res;
}


}} // namespaces
