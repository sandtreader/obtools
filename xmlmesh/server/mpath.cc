//==========================================================================
// ObTools::XMLMesh:Server mpath.cc
//
// Implementation of XMLMesh message path
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
  const vector<string> segs = Text::split(s, '|');
  for(vector<string>::const_iterator p = segs.begin(); p!=segs.end(); ++p)
    push(*p);
}

//------------------------------------------------------------------------
// Push a path level integer
void MessagePath::push(int n)
{
  push(Text::itos(n));
}

//------------------------------------------------------------------------
// Pop a path level integer
int MessagePath::popi()
{
  return Text::stoi(pop());
}

//------------------------------------------------------------------------
// Generate a | delimited string (top last)
string MessagePath::to_string() const
{
  string s;
  bool first=true;
  for(deque<string>::const_iterator p=path.begin();
      p!=path.end();
      p++, first=false)
  {
    if (!first) s+='|';
    s+=*p;
  }

  return s;
}

}} // namespaces




