//==========================================================================
// ObTools::Misc: proplist.cc
//
// Property list
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-misc.h"
#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace Misc {

//------------------------------------------------------------------------
// Constructor from delimited string - e.g. foo=1,bar=2
PropertyList::PropertyList(const string& str, char sep)
{
  // Split on seperator, with canonicalisation of values
  vector<string> bits = Text::split(str, sep, true);
  for(vector<string>::iterator p = bits.begin(); p!=bits.end(); ++p)
  {
    // Split individual bit on = and trim
    vector<string> bobs = Text::split(*p, '=', true, 2);
    if (bobs.size() == 2) add(bobs[0], bobs[1]);
  }
}

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
// Variables are terminated by non-alphanum or ";"
// $ and ; can be escaped as $$ and $;
// e.g.
//  $name   -> fred
//  $name;s -> freds
//  $name$name -> fredfred
//  $name$;s -> fred;s
//  $$name  -> $fred
// Unset variables are not substituted
string PropertyList::interpolate(const string& text)
{
  string result;

  for(string::const_iterator p = text.begin(); p!=text.end();)
  {
    char c = *p++;

retry:
    if (c == '$')
    {
      string var;
      c=0;

      // Read to nonalphanum (or '_')
      while (p!=text.end())
      {
	c = *p++;
	if (isalnum(c) || c=='_')
	{
	  var += c;
	  c=0;  // Don't reuse if at end
	}
	else break;
      }

      // If no word, must be non-alpha at start
      if (var.empty())
      {
	switch (c)
	{
	  case '$':  // Escaped $, or $ at end
	  case ';':  // Escaped ;
	    result += c;
	    break;

          default:
	    result += '$';  // Reinsert original
	    result += c;
	    break;
	}
      }
      else
      {
	// Lookup word
	const_iterator vp = find(var);
	if (vp!=end())
	{
	  // Known word - replace with value
	  result += vp->second;
	}
	else
	{
	  // Unknown - replace $ and word, and semi if used
	  result += '$';
	  result += var;
	  if (c==';') result += c;
	}

	// Retry with this character (may be a $)
	if (c && c!=';') goto retry;
      }
    }
    else
    {
      // Any other character goes straight through
      result += c;
    }
  }
  
  return result;
}


}} // namespaces
