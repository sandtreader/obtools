//==========================================================================
// ObTools::Text: subst.cc
//
// String subst functions
//
// Copyright (c) 2004 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace Text {

//==========================================================================
// Replace functions
// Global String replace - equivalent to s/old/rep/g
// Returns whether any replaced
// Case sensitive
// Handles case where new string includes old
bool subst(string& text, const string& old, const string& rep)
{
  string::size_type p=0;
  string::size_type old_l = old.size();
  string::size_type rep_l = rep.size();
  bool found = false;

  while (p<text.size() && (p=text.find(old,p))!=string::npos) 
  {
    text.replace(p,old_l,rep);
    p+=rep_l;
    found = true;
  }

  return found;
}

}} // namespaces
