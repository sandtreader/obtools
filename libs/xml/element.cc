//==========================================================================
// ObTools::XML: element.cc
//
// XML Element structure
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "xml.h"
#include <ctype.h>

using namespace ObTools::XML;

//--------------------------------------------------------------------------
// Non-element marker
Element Element::none("!NONE!");

//--------------------------------------------------------------------------
// Write to a given stream with given indent
void Element::write_indented(int indent, ostream &s) const
{
  for(int i=0; i<indent; i++) s<<' ';

  //Name indicates true element
  if (name.size())
  {
    s << '<' << name;

    //Output attributes
    for(map<string,string>::const_iterator p=attrs.begin();
	p!=attrs.end();
	p++)
    {
      xmlchar delim;
      const string &v = p->second;
      bool escquote=false;

      // Decide whether value contains ' or " or both
      if (v.find('"') != string::npos)
      {
	//It has a " in it - see if it also contains a '
	if (v.find('\'') != string::npos)
	{
	  //Oops - it has both.  Better escape the quote, then
	  escquote=true;
	}
	else
	{
	  //We'll be OK by swapping to using ' as delimiter
	  delim = '\'';
	}
      }
      else
      {
	//That's fine, we can use " as delimiter
	delim = '"';
      }

      //But we must escape &, < and > as well
      s << ' ' << p->first << "=" << delim << escape(v, escquote) << delim;
    } 

    //Output sub-elements if any
    if (children.empty())
    {
      //Could have an 'optimised' content string, though
      if (content.size())
      {
	//String it all on one line
	s << '>' << content << "</" << name << '>' << endl;
      }
      else
      {
	//No, it's really empty
	s << "/>" << endl;
      }
    }
    else
    {
      s << '>' << endl;

      for(list<Element *>::const_iterator p=children.begin();
	  p!=children.end();
	  p++)
      {
	(*p)->write_indented(indent+2, s);
      }

      for(int i=0; i<indent; i++) s<<' ';
      s << "</" << name << '>' << endl;
    }
  }
  else
  {
    //Textual data in content - escape for &, < and >
    s << escape(content, false) << endl;
  }
}

//--------------------------------------------------------------------------
// Escape a string for &, < and >, and optionally "
// Note:  We don't strictly have to escape > since we never issue a CDATA
// marked section, but it's tidier and causes less risk of breaking dumb
// parsers
string Element::escape(const string &v, bool escquote) const
{
  string r;

  for(string::const_iterator p=v.begin(); p!=v.end(); p++)
  {
    char c=*p;
    switch (c)
    {
      case '<':
	r+="&lt;";
	break;

      case '>':
	r+="&gt;";
	break;

      case '&':
	r+="&amp;";
	break;
	
      case '"':
	if (escquote)
	  r+="&quot;";
	else
	  r+=c;
	break;

      default:
	r+=c;
    }
  }
  return r;
}

//--------------------------------------------------------------------------
// Write to a given stream
void Element::write_to(ostream &s) const
{
  s << "<?xml version=\"1.0\"?>" << endl;
  write_indented(0, s);
}

//--------------------------------------------------------------------------
// Optimise for ease of access so a single content child element becomes
// a simple content string within us - and kill the child
void Element::optimise()
{
  if (children.size()==1 && children.back()->name.empty())
  {
    content = children.back()->content;
    delete children.back();
    children.pop_back();
  }
}

//--------------------------------------------------------------------------
// Find first (or only) child element of given name
// Returns Element::none if none
Element& Element::get_child(const string& name)
{
  for(list<Element *>::iterator p=children.begin();
      p!=children.end();
      p++)
  {
    if ((*p)->name==name) return **p;
  }
  return Element::none;
}

//--------------------------------------------------------------------------
// Find all child elements of given name
// Returns list of pointers
list<Element *> Element::get_children(const string& name)
{
  list<Element *>l;
  for(list<Element *>::iterator p=children.begin();
      p!=children.end();
      p++)
  {
    if ((*p)->name==name) l.push_back(*p);
  }
  return l;
}

//--------------------------------------------------------------------------
// Find all descendant elements of given name - recursive
// Returns flat list of pointers
list<Element *> Element::get_descendants(const string& name)
{
  list<Element *>l;
  append_descendants(name, l);
  return l;
}

//--------------------------------------------------------------------------
// Dump all descendant elements of given name into given list
void Element::append_descendants(const string& name, list<Element *>& l)
{
  for(list<Element *>::iterator p=children.begin();
      p!=children.end();
      p++)
  {
    if ((*p)->name==name) 
      l.push_back(*p);

    //Look for descendants, even within a match
    (*p)->append_descendants(name, l);
  }
}

//--------------------------------------------------------------------------
// Get an attribute of the given name
// Returns attribute value
// Defaults to default value given (or "") if not present
// This exists to avoid modifying the attribute when using attrs["foo"]
// when foo doesn't exist (a completely stupid specification of [], IMHO)
string Element::get_attr(const string& name, const char *def)
{
  map<string,string>::iterator p=attrs.find(name);
  if (p!=attrs.end())
    return p->second;
  else
    return def;
}

//--------------------------------------------------------------------------
// Destructor
// Recursively destroys children
Element::~Element()
{
  for(list<Element *>::iterator p=children.begin();
      p!=children.end();
      p++)
    delete *p;
}

//------------------------------------------------------------------------
// >> operator to write to ostream
ostream& ObTools::XML::operator<<(ostream& s, const Element& e) 
{
  e.write_to(s);
  return s;
}




