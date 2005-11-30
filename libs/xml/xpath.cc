//==========================================================================
// ObTools::XML: xpath.cc
//
// Micro XPath processor
// Only handles very basic x/y/z[3]/@foo syntax (for now!)
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xml.h"
#include "unistd.h"

using namespace ObTools::XML;

//------------------------------------------------------------------------
// Element list fetch - all elements matching final child step.
// Only first (or n'th) element of intermediate steps is used - 
// cousins are not merged!
list<Element *> XPathProcessor::get_elements(const string& path)
{
  list<Element *> el;
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
      el.push_back(current);
      return el;
    }

    // Locate next step delimiter, or end
    string::size_type delim = path.find('/', pos);
    if (delim == string::npos) delim=size;

    string::size_type name_end;
    int count = -1;

    // Look for [ within step indicating count
    string::size_type bopen = path.find('[', pos);
    if (bopen != string::npos && bopen<delim)
    {
      // Find closing ] and get count inside
      string::size_type bclose = path.find(']', bopen);
      if (bclose != string::npos && bclose<delim && bclose-bopen>1)
      {
	string ns(path, bopen+1, bclose-bopen-1);
	count = atoi(ns.c_str())-1;  // We count from zero
      }

      name_end = bopen;
    }
    else
    {
      name_end = delim;
    }
    
    // Extract name
    string name(path, pos, name_end-pos);

    // Get elements to return if this is the last one and not counted
    if (delim == size && count<0)
    {
      // Last step - get all children of this name
      return current->get_children(name);
    }

    // Need to find a single element, either to return or continue with
    // Get first or counted child
    Element& child = current->get_child(name, count<0?0:count);
    if (child.valid())
      current = &child;
    else
      return el;

    // If at end, return this one, otherwise loop
    if (delim == size)
    {
      el.push_back(current);
      return el;
    }

    // Move to next step
    pos = delim+1;
  }

  return el;
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

//--------------------------------------------------------------------------
// Real value fetch
// Defaults to default value given (or 0.0) if not present
// Returns 0.0 if present but bogus
double XPathProcessor::get_value_real(const string& path, double def)
{
  string v = get_value(path);
  if (!v.empty()) return atof(v.c_str());
  return def;
}

//--------------------------------------------------------------------------
// Delete the element(s) at the given path
void XPathProcessor::delete_elements(const string& path)
{
  list<Element *> els = get_elements(path);
  for(list<Element *>::iterator p = els.begin(); p!=els.end(); ++p)
  {
    Element *e = *p;
    e->detach();
    delete e;
  }
}

//--------------------------------------------------------------------------
// Add an element below the given path
// Takes the element and attaches to given path
void XPathProcessor::add_element(const string& path, Element *ne)
{
  Element *e = get_element(path);
  if (e) e->add(ne);
}

//--------------------------------------------------------------------------
// Replace an element at the given path with the new one
// Takes the element and attaches to given path, detachs and deletes the old
void XPathProcessor::replace_element(const string& path, Element *ne)
{
  Element *e = get_element(path);
  if (e)
  {
    e->replace_with(ne);
    delete e;
  }
}


