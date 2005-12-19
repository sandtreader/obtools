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
#include <sstream>

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

//------------------------------------------------------------------------
// Set value either attribute or content of single (first) element
// Returns whether value was settable
// Can only set content or attributes of existing elements - use add_element
// to create new ones. 
bool XPathProcessor::set_value(const string& path, const string& value)
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
      // Set attribute
      string aname(path, att+1);
      e->set_attr(aname, value);
      return true;
    }
  }
  else
  {
    // Use path as is and set content
    Element *e = get_element(path);
    if (e)
    {
      e->content = value;
      return true;
    }
  }

  // Failed 
  return false;
}

//--------------------------------------------------------------------------
// Boolean value set
// Sets value to 'yes' or 'no'
bool XPathProcessor::set_value_bool(const string& path, bool value)
{
  return set_value(path, value?"yes":"no");
}

//--------------------------------------------------------------------------
// Integer value set
bool XPathProcessor::set_value_int(const string& path, int value)
{
  ostringstream oss;
  oss << value;
  return set_value(path, oss.str());
}

//--------------------------------------------------------------------------
// Real value set
bool XPathProcessor::set_value_real(const string& path, double value)
{
  ostringstream oss;
  oss << value;
  return set_value(path, oss.str());
}

//--------------------------------------------------------------------------
// Delete the element(s) at the given path
// Returns whether any such element existed
bool XPathProcessor::delete_elements(const string& path)
{
  list<Element *> els = get_elements(path);
  bool any = false;
  for(list<Element *>::iterator p = els.begin(); p!=els.end(); ++p)
  {
    Element *e = *p;
    e->detach();
    delete e;
    any = true;
  }

  return any;
}

//--------------------------------------------------------------------------
// Add an element below the given path
// Takes the element and attaches to given path
// Returns whether the parent element existed
bool XPathProcessor::add_element(const string& path, Element *ne)
{
  Element *e = get_element(path);
  if (e)
  {
    e->add(ne);
    return true;
  }
  else return false;
}

//--------------------------------------------------------------------------
// Add an element below the given path with given name
// Creates empty element of given name below path
// Returns new element if created, or 0 if parent didn't exist
Element *XPathProcessor::add_element(const string& path, const string& name)
{
  Element *e = get_element(path);
  if (e)
  {
    Element *child = new Element(name);
    e->add(child);
    return child;
  }
  else return 0;
}

//--------------------------------------------------------------------------
// Ensure the given element path exists
// Creates empty elements to fulfill the entire path if they don't already
// exist.  Uses the first of any given name for path if more than one
// Returns pointer to eventual child element (cannot fail)
Element *XPathProcessor::ensure_path(const string& path)
{
  // See if it already exists
  Element *e = get_element(path);
  if (e) return e;

  // Then we must create it - first make sure the parent path exists
  // Find last / (if any)
  string::size_type delim = path.rfind('/');
  Element *parent = &root;
  string child_name = path;

  if (delim != string::npos)  // Has a /
  {
    if (delim)  // Ignore leading /
    {
      string parent_path(path, 0, delim);
      parent = ensure_path(parent_path);
    }
    child_name = string(path, delim+1);
  }

  // Create element here
  Element *child = new Element(child_name);
  parent->add(child);
  return child;
}

//--------------------------------------------------------------------------
// Replace an element at the given path with the new one
// Takes the element and attaches to given path, detachs and deletes the old
// Returns whether the old element existed
bool XPathProcessor::replace_element(const string& path, Element *ne)
{
  Element *e = get_element(path);
  if (e)
  {
    e->replace_with(ne);
    delete e;
    return true;
  }
  else return false;
}


