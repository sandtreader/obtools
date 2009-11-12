//==========================================================================
// ObTools::Text: subst.cc
//
// String subst functions
//
// Copyright (c) 2004 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace Text {

//==========================================================================
// Replace functions
// Global String replace - equivalent to s/old/rep/g
// Case sensitive
// Handles case where new string includes old
string subst(string text, const string& old, const string& rep)
{
  string::size_type p=0;
  string::size_type old_l = old.size();
  string::size_type rep_l = rep.size();

  while (p<text.size() && (p=text.find(old,p))!=string::npos) 
  {
    text.replace(p,old_l,rep);
    p+=rep_l;
  }

  return text;
}

}} // namespaces
