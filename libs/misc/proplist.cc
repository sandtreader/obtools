//==========================================================================
// ObTools::Misc: proplist.cc
//
// Property list
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-misc.h"

namespace ObTools { namespace Misc {

//--------------------------------------------------------------------------
// Dump contents
void PropertyList::dump(ostream& s, const string& prefix,
			const string& separator) const
{
  for(const_iterator p = begin(); p!=end(); ++p)
    s << prefix << p->first << separator << p->second << endl; 
}

//------------------------------------------------------------------------
// << operator to write PropertyList to ostream
ostream& operator<<(ostream& s, const PropertyList& pl) 
{ 
  pl.dump(s); 
  return s;
}

}} // namespaces
