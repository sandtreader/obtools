//==========================================================================
// ObTools::XML: config.cc
//
// XML Configuration file reader
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xml.h"
#include <fstream>

using namespace ObTools::XML;

//------------------------------------------------------------------------
// Read configuration file
// Returns whether successful
// ename is the expected root element name - fails if wrong
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
	cerr << "Bad XML in config file\n";
	return false;
      }

      Element& root = parser.get_root();
      if (root.name != ename)
      {
	cerr << "Bad root in config file - expected <" << ename 
	     << ">, got <" << root.name << ">\n";
	return false;
      }

      return true;
    }
  }

  // Nothing will open
  cerr << "Can't find config file\n";
  return false;
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
bool Configuration::get_value_bool(const string& path, bool def=false)
{ 
  XPathProcessor xpath(get_root());
  return xpath.get_value_bool(path, def); 
}

//--------------------------------------------------------------------------
// Integer value fetch
// Defaults to default value given (or 0) if not present
// Returns 0 if present but bogus
int Configuration::get_value_int(const string& path, int def=0)
{ 
  XPathProcessor xpath(get_root());
  return xpath.get_value_int(path, def); 
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
  

