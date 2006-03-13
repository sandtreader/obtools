//==========================================================================
// ObTools::Misc: proplist.cc
//
// Property list
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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

//------------------------------------------------------------------------
// Variable interpolation of property list into a string
// Replaces (e.g.) $var with value from property list
// You can (optionally) specify variable escape start ('$'), 
// escaped escape ('$$') and variable end string ('')
// Unset variables are not substituted
string interpolate(const string& text,
		   const string& start="$",
		   const string& escape="$$",
		   const string& end="")
{

}


}} // namespaces
