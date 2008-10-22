//==========================================================================
// ObTools::XML: config.cc
//
// XML Configuration file reader
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xml.h"
#include <unistd.h>
#include <stdio.h>
#include <fstream>
#include <errno.h>

#if defined(__WIN32__)
#include <windows.h>
#endif

using namespace ObTools::XML;

//------------------------------------------------------------------------
// Read configuration file
// Returns whether successful
// If specified, ename is the expected root element name - fails if wrong
bool Configuration::read(const string& ename)
{
  // Run through list of filenames to find one that will open
  for(list<string>::iterator p = filenames.begin();
      p!=filenames.end();
      p++)
  {
    ifstream f(p->c_str());
    if (f)
    {
      try
      {
	f >> parser;
      }
      catch (ParseFailed)
      {
	serr << "Bad XML in config file\n";
	return false;
      }

      if (!ename.empty())
      {
	Element& root = parser.get_root();
	if (root.name != ename)
	{
	  serr << "Bad root in config file - expected <" << ename 
	       << ">, got <" << root.name << ">\n";
	  return false;
	}
      }

      return true;
    }
  }

  // Nothing will open - but don't complain, they might be expecting
  // this
  return false;
}

//------------------------------------------------------------------------
// Reload configuration file as the same element as before
// Returns whether successful
bool Configuration::reload()
{
  string ename = get_root().name;
  return read(ename);
}

//------------------------------------------------------------------------
// Element list fetch - all elements matching final child step.
// Only first element of intermediate steps is used - list is not merged!
list<Element *> Configuration::get_elements(const string& path)
{
  XPathProcessor xpath(get_root());
  return xpath.get_elements(path);
}

//------------------------------------------------------------------------
// Single element fetch - first of list, if any, or 0
Element *Configuration::get_element(const string& path)
{
  XPathProcessor xpath(get_root());
  return xpath.get_element(path);
}

//------------------------------------------------------------------------
// XPath value fetch - either attribute or content of single (first) element
// Returns def if anything not found
// Note all get_value methods still work, and return def, if file read fails
string Configuration::get_value(const string& path, const string& def)
{ 
  XPathProcessor xpath(get_root());
  return xpath.get_value(path, def); 
}

//--------------------------------------------------------------------------
// XPath Boolean value fetch
// Defaults to default value given (or false) if not present
// Recognises words beginning [TtYy] as true, everything else is false
bool Configuration::get_value_bool(const string& path, bool def)
{ 
  XPathProcessor xpath(get_root());
  return xpath.get_value_bool(path, def); 
}

//--------------------------------------------------------------------------
// Integer value fetch
// Defaults to default value given (or 0) if not present
// Returns 0 if present but bogus
int Configuration::get_value_int(const string& path, int def)
{ 
  XPathProcessor xpath(get_root());
  return xpath.get_value_int(path, def); 
}

//--------------------------------------------------------------------------
// Integer hex value fetch
// Defaults to default value given (or 0) if not present
// Returns 0 if present but bogus
int Configuration::get_value_hex(const string& path, int def)
{ 
  XPathProcessor xpath(get_root());
  return xpath.get_value_hex(path, def); 
}

//--------------------------------------------------------------------------
// 64-bit integer value fetch
// Defaults to default value given (or 0) if not present
// Returns 0 if present but bogus
uint64_t Configuration::get_value_int64(const string& path, uint64_t def)
{ 
  XPathProcessor xpath(get_root());
  return xpath.get_value_int64(path, def); 
}

//--------------------------------------------------------------------------
// 64-bit integer hex value fetch
// Defaults to default value given (or 0) if not present
// Returns 0 if present but bogus
uint64_t Configuration::get_value_hex64(const string& path, uint64_t def)
{ 
  XPathProcessor xpath(get_root());
  return xpath.get_value_hex64(path, def); 
}

//--------------------------------------------------------------------------
// Real value fetch
// Defaults to default value given (or 0.0) if not present
// Returns 0.0 if present but bogus
double Configuration::get_value_real(const string& path, double def)
{ 
  XPathProcessor xpath(get_root());
  return xpath.get_value_real(path, def); 
}

//------------------------------------------------------------------------
// XPath list-of-values fetch
// Returns contents of all elements matching XPath
list<string> Configuration::get_values(const string& path)
{ 
  XPathProcessor xpath(get_root());
  list<Element *> elems = xpath.get_elements(path);
  list<string> strings;

  for(list<Element *>::iterator p = elems.begin();
      p!=elems.end();
      p++)
    strings.push_back((*p)->get_content());

  return strings;
}
  
//------------------------------------------------------------------------
// XPath map fetch
// Returns string->string map of all element matching given XPath,
// using given attribute name as key, content as value
map<string, string> Configuration::get_map(const string& path,
					   const char *name_attr)
{ 
  XPathProcessor xpath(get_root());
  list<Element *> elems = xpath.get_elements(path);
  map<string, string> values;

  for(list<Element *>::iterator p = elems.begin();
      p!=elems.end();
      p++)
  {
    Element *e = *p;
    if (e->has_attr(name_attr))
      values[e->get_attr(name_attr)] = e->get_content();
  }

  return values;
}

//------------------------------------------------------------------------
// XPath value set - either attribute or content of single (first) element
// Returns whether value was settable
// Can only set content or attributes of existing elements - use add_element
// to create new ones. 
bool Configuration::set_value(const string& path, const string& value)
{ 
  XPathProcessor xpath(get_root());
  return xpath.set_value(path, value); 
}

//--------------------------------------------------------------------------
// XPath Boolean value set - sets to 'yes' or 'no'
bool Configuration::set_value_bool(const string& path, bool value)
{ 
  XPathProcessor xpath(get_root());
  return xpath.set_value_bool(path, value); 
}

//--------------------------------------------------------------------------
// Integer value set
bool Configuration::set_value_int(const string& path, int value)
{ 
  XPathProcessor xpath(get_root());
  return xpath.set_value_int(path, value); 
}

//--------------------------------------------------------------------------
// Integer value set to hex
bool Configuration::set_value_hex(const string& path, int value)
{ 
  XPathProcessor xpath(get_root());
  return xpath.set_value_hex(path, value); 
}

//--------------------------------------------------------------------------
// 64-bit integer value set
bool Configuration::set_value_int64(const string& path, uint64_t value)
{ 
  XPathProcessor xpath(get_root());
  return xpath.set_value_int64(path, value); 
}

//--------------------------------------------------------------------------
// 64-bit integer value set to hex
bool Configuration::set_value_hex64(const string& path, uint64_t value)
{ 
  XPathProcessor xpath(get_root());
  return xpath.set_value_hex64(path, value); 
}

//--------------------------------------------------------------------------
// Real value set
bool Configuration::set_value_real(const string& path, double value)
{ 
  XPathProcessor xpath(get_root());
  return xpath.set_value_real(path, value); 
}

//--------------------------------------------------------------------------
// Delete the element(s) at the given path
// Returns whether any such element existed
bool Configuration::delete_elements(const string& path)
{
  XPathProcessor xpath(get_root());
  return xpath.delete_elements(path); 
}

//--------------------------------------------------------------------------
// Add an element below the given path
// Takes the element and attaches to given path
// Returns whether the parent element existed
bool Configuration::add_element(const string& path, Element *ne)
{
  XPathProcessor xpath(get_root());
  return xpath.add_element(path, ne); 
}

//--------------------------------------------------------------------------
// Add an element below the given path with given name
// Creates empty element of given name below path
// Returns new element if created, or 0 if parent didn't exist
Element *Configuration::add_element(const string& path, const string& name)
{
  XPathProcessor xpath(get_root());
  return xpath.add_element(path, name); 
}

//--------------------------------------------------------------------------
// Ensure the given element path exists
// Creates empty elements to fulfill the entire path if they don't already
// exist.  Uses the first of any given name for path if more than one
// Returns pointer to eventual child element (cannot fail)
Element *Configuration::ensure_path(const string& path)
{
  XPathProcessor xpath(get_root());
  return xpath.ensure_path(path); 
}

//--------------------------------------------------------------------------
// Replace an element at the given path with the new one
// Takes the element and attaches to given path, detachs and deletes the old
// Returns whether the old element existed
bool Configuration::replace_element(const string& path, Element *ne)
{
  XPathProcessor xpath(get_root());
  return xpath.replace_element(path, ne); 
}

//------------------------------------------------------------------------
// Replace the root (if any) with a new one of the given name
// Returns the new root
Element *Configuration::replace_root(const string& name)
{
  XML::Element *e = new XML::Element(name);
  parser.replace_root(e);
  return e;
}
  
//------------------------------------------------------------------------
// Update file from changes made to in-memory document
// Writes back to first (or only) file in filename list
// Update is atomic with rename
// Returns whether successful
// NB: All comments are lost!
bool Configuration::write()
{
  string fn = filenames.front();
  if (fn.empty())
  {
    serr << "Config: no filename available for write\n";
    return false;
  }

  // Create and open temporary file
  string tfn = fn+"~new";

  ofstream f(tfn.c_str());
  if (f)
  {
    f << parser.get_root();
    f.close();

    // Attempt atomic update 
#if defined(__WIN32__)
    // MoveFileEx in NT+ may be atomic (not clear from docs)
    if (MoveFileEx(tfn.c_str(), fn.c_str(), MOVEFILE_REPLACE_EXISTING)) 
      return true;

    // Fall back to try non-atomic delete/rename (95/98)
    ::unlink(fn.c_str());  // Doesn't matter if it fails
#endif

    if (!::rename(tfn.c_str(), fn.c_str())) return true;


    serr << "Config: Can't rename " << tfn << " to " << fn << ": " 
	 << strerror(errno) << endl;

    unlink(tfn.c_str());
  }
  else serr << "Config: can't open " << tfn << " for update: "
	    << strerror(errno) << endl;
   
  return false;
}

