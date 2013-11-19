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
#include <sstream>

namespace ObTools { namespace Misc {

//------------------------------------------------------------------------
// Constructor from delimited string - e.g. foo=1,bar=2 and optional
// quote character, in which the delimiter is ignored and whitespace
// preserved - quotes are removed from values
PropertyList::PropertyList(const string& str, char sep, char quote)
{
  string::size_type p = 0;
  for(;;)
  {
    // Find first =
    string::size_type ep = str.find('=', p);
    if (ep == string::npos) break;  // No equals left

    // Find first separator
    string::size_type sp = str.find(sep, p);

    // If separator before =, it's a name with no value
    if (sp != string::npos && sp < ep)
    {
      // Skip it
      p = sp+1;
      continue;
    }

    // Get property name
    string name(str, p, ep-p);
    name = Text::canonicalise_space(name);

    // Find next quote
    string::size_type qp = str.find(quote, ep+1);
    
    // Get sp fenced to end, for convenience
    string::size_type spf = (sp == string::npos)?str.size():sp;

    // If quote exists before sep we need to use it
    if (qp != string::npos && qp<spf)
    {
      // Find the second quote
      string::size_type qp2 = str.find(quote, qp+1);
      if (qp2 != string::npos)
      {
	// That's the value between them - use it, unmodified
	add(name, string(str, qp+1, qp2-qp-1));
	
	// Check for next separator after closing quote for next go
	sp = str.find(sep, qp2+1);
      }
      else
      {
	// Unclosed quote - assume it stops at sep to recover
	add(name, string(str, qp+1, spf-qp-1));
      }
    }
    else
    {
      // Use the separator as is, with trimming of value
      string value(str, ep+1, spf-ep-1);
      value = Text::canonicalise_space(value);
      add(name, value);
    }

    // Move past separator (if any)
    if (sp == string::npos)
      break;
    else
      p = sp+1;
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

//--------------------------------------------------------------------------
// Convert to delimited string
string PropertyList::str(char sep, char quote)
{
  ostringstream oss;
  for(const_iterator p = begin(); p!=end(); ++p)
  {
    if (p!=begin()) oss << sep;
    oss << p->first << "=";
    if (p->second.find(sep) != string::npos)
      oss << quote << p->second << quote;
    else
      oss << p->second;
  }

  return oss.str();
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
string PropertyList::interpolate(const string& text) const
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
