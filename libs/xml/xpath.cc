//==========================================================================
// ObTools::XML: xpath.cc
//
// Micro XPath processor
// Only handles very basic x/y/z/@foo syntax (for now!)
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xml.h"
using namespace ObTools::XML;

//------------------------------------------------------------------------
// Element list fetch - all elements matching final child step.
// Only first element of intermediate steps is used - cousins are not merged!
list<Element *> XPathProcessor::get_elements(const string& path)
{
  list<Element *> empty;  // Fallback for no results
  Element *current = &root;
  string::size_type pos=0;
  string::size_type size=path.size();

  while (pos<=size)
  {
    // Skip over / (if any)
    if (pos<size && path[pos] == '/') pos++;

    // If it stops here, it's the root they want
    if (pos==size)
    {
      empty.push_back(current);  // OK, it's not quite so empty any more
      return empty;
    }

    // Locate next step delimiter, or end
    string::size_type delim = path.find('/', pos);
    if (delim == string::npos) delim=size;

    // Extract step
    string step(path, pos, delim-pos);

    // Get elements - single if intermediate, all if last
    if (delim == size)
    {
      // Last step - get all children of this name
      return current->get_children(step);
    }
    else
    {
      // Intermediate - just move to first of this name
      Element& child = current->get_child(step);
      if (child.valid())
	current = &child;
      else
	return empty;
    }
    // Move to next step
    pos = delim+1;
  }

  return empty;
}

//------------------------------------------------------------------------
// Single element fetch - first of list, if any, or 0
Element *XPathProcessor::get_element(const string& path)
{
  list<Element *> l = get_elements(path);
  if (l.size())
    return l.front();
  else
    return 0;
}

//------------------------------------------------------------------------
// Value fetch - either attribute or content of single (first) element
// Returns def if anything not found
string XPathProcessor::get_value(const string& path, const string& def)
{
  Element *final;

  // See if it's an attribute step at the end
  string::size_type att = path.rfind('@');

  if (att != string::npos)
  {
    // Strip off attribute step to get final element
    string spath(path, 0, att);
    Element *e = get_element(spath);
    if (e)
    {
      // Get attribute
      string aname(path, att+1);
      return e->get_attr(aname, def);
    }
  }
  else
  {
    // Use path as is and return final content (aggregated)
    Element *e = get_element(path);
    if (e) return e->get_content();
  }

  // Failed - return default
  return def;
}

//--------------------------------------------------------------------------
// Boolean value fetch
// Defaults to default value given (or false) if not present
// Recognises words beginning [TtYy] as true, everything else is false
bool XPathProcessor::get_value_bool(const string& path, bool def)
{
  string v = get_value(path);
  if (!v.empty())
  {
    switch(v[0])
    {
      case 'T': case 't':
      case 'Y': case 'y':
	return true;

      default:
	return false;
    }
  }

  return def;
}

//--------------------------------------------------------------------------
// Integer value fetch
// Defaults to default value given (or 0) if not present
// Returns 0 if present but bogus
int XPathProcessor::get_value_int(const string& path, int def)
{
  string v = get_value(path);
  if (!v.empty()) return atoi(v.c_str());
  return def;
}


