//==========================================================================
// ObTools::XMLMesh:Server mpath.cc
//
// Implementation of XMLMesh message path
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "server.h"
#include "ot-log.h"
#include <sstream>

namespace ObTools { namespace XMLMesh { 

//------------------------------------------------------------------------
// Constructor from string 
// String is '|' separated, top last
MessagePath::MessagePath(const string& s)
{
  string seg;
  // Split into segments on '|', top last
  for(string::const_iterator p = s.begin(); p!=s.end(); p++)
  {
    char c=*p;
    if (c=='|')
    {
      push(seg);
      seg="";
    }
    else seg+=c;
  }

  if (seg.size()) push(seg);
}

//------------------------------------------------------------------------
// Push a path level integer 
void MessagePath::push(int n)
{
  ostringstream oss;
  oss << n;
  push(oss.str());
}

//------------------------------------------------------------------------
// Pop a path level integer
int MessagePath::popi()
{
  return atoi(pop().c_str());
}

//------------------------------------------------------------------------
// Generate a | delimited string (top last)
string MessagePath::to_string()
{
  string s;
  bool first=true;
  for(deque<string>::iterator p=path.begin(); 
      p!=path.end(); 
      p++, first=false)
  {
    if (!first) s+='|';
    s+=*p;
  }
    
  return s;
}

}} // namespaces




