//==========================================================================
// ObTools::XML: element.cc
//
// XML Element structure
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xml.h"
#include <ctype.h>
#include <sstream>

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
// Find first (or only) child element of any type
// Returns Element::none if none
// Const and non-const implementations
const Element& Element::get_child() const
{
  list<Element *>::const_iterator p=children.begin();
  if (p!=children.end())
    return **p;
  else
    return Element::none;
}

Element& Element::get_child()
{
  list<Element *>::iterator p=children.begin();
  if (p!=children.end())
    return **p;
  else
    return Element::none;
}

//--------------------------------------------------------------------------
// Find first (or only) child element of given name
// Returns Element::none if none
// Const and non-const implementations
const Element& Element::get_child(const string& ename) const
{
  for(list<Element *>::const_iterator p=children.begin();
      p!=children.end();
      p++)
  {
    if ((*p)->name==ename) return **p;
  }
  return Element::none;
}

Element& Element::get_child(const string& ename)
{
  for(list<Element *>::iterator p=children.begin();
      p!=children.end();
      p++)
  {
    if ((*p)->name==ename) return **p;
  }
  return Element::none;
}

//--------------------------------------------------------------------------
// Find first (or only) descendant of given name
// Returns Element::none if there isn't one 
// (Like get_child() but ignoring intervening cruft)
// Const and non-const implementations
const Element &Element::get_descendant(const string& ename) const
{
  for(list<Element *>::const_iterator p=children.begin();
      p!=children.end();
      p++)
  {
    const Element& se = **p;
    if (se.name == ename) return se;

    //Not child, but ask it to search below itself
    const Element& sse = se.get_descendant(ename);
    if (sse.valid()) return sse;
  }

  return Element::none;
}

Element &Element::get_descendant(const string& ename)
{
  for(list<Element *>::iterator p=children.begin();
      p!=children.end();
      p++)
  {
    Element& se = **p;
    if (se.name == ename) return se;

    //Not child, but ask it to search below itself
    Element& sse = se.get_descendant(ename);
    if (sse.valid()) return sse;
  }

  return Element::none;
}

//--------------------------------------------------------------------------
// Find all child elements of given name
// Returns list of pointers
// Const and non-const implementations
list<const Element *> Element::get_children(const string& ename) const
{
  list<const Element *>l;
  for(list<Element *>::const_iterator p=children.begin();
      p!=children.end();
      p++)
  {
    if ((*p)->name==ename) l.push_back(*p);
  }
  return l;
}

list<Element *> Element::get_children(const string& ename)
{
  list<Element *>l;
  for(list<Element *>::iterator p=children.begin();
      p!=children.end();
      p++)
  {
    if ((*p)->name==ename) l.push_back(*p);
  }
  return l;
}

//--------------------------------------------------------------------------
// Find all descendant elements of given name - recursive
// Returns flat list of pointers
// Prunes tree walk at 'prune' tags if set - use for recursive structures
// where you want to deal with each level independently
// Const and non-const implementations
list<const Element *> Element::get_descendants(const string& ename,
					       const string& prune) const
{
  list<const Element *>l;
  append_descendants(ename, prune, l);
  return l;
}

list<Element *> Element::get_descendants(const string& ename,
					 const string& prune)
{
  list<Element *>l;
  append_descendants(ename, prune, l);
  return l;
}

//--------------------------------------------------------------------------
// Dump all descendant elements of given name into given list
// Prune walk at elements matching 'prune'
// Ename and prune can be the same - then returns only first level of 
// <ename>s, not <ename>s within <ename>s
// Const and non-const implementations
void Element::append_descendants(const string& ename, const string& prune,
				 list<const Element *>& l) const
{
  for(list<Element *>::const_iterator p=children.begin();
      p!=children.end();
      p++)
  {
    if ((*p)->name==ename) 
      l.push_back(*p);

    //Look for descendants, even within a match
    if ((*p)->name!=prune)
      (*p)->append_descendants(ename, prune, l);
  }
}

void Element::append_descendants(const string& ename, const string& prune,
				 list<Element *>& l)
{
  for(list<Element *>::iterator p=children.begin();
      p!=children.end();
      p++)
  {
    if ((*p)->name==ename) 
      l.push_back(*p);

    //Look for descendants, even within a match
    if ((*p)->name!=prune)
      (*p)->append_descendants(ename, prune, l);
  }
}

//--------------------------------------------------------------------------
// Get all direct child text content accumulated into one string
// Returns optimised content if available, otherwise iterates children
// collecting text from data elements
// Strings from separate elements are separately with '\n'
string Element::get_content() const
{
  // Return content directly if available
  if (content != "") return content;

  // Otherwise iterate all elements
  string s;
  for(list<Element *>::const_iterator p=children.begin();
      p!=children.end();
      p++)
  {
    const Element &e=**p;

    // Look for absence of tag rather than presence of content - this
    // might be optimised and we don't want to collect second-level stuff
    if (e.name.empty() && !e.content.empty()) 
    {
      s+=(*p)->content;
      s+='\n';
    }
  }

  return s;
}

//--------------------------------------------------------------------------
// Get all text content from the entire tree accumulated into one string
// Returns optimised content if available, otherwise iterates children
// collecting text from data elements, and recursing into subchildren
// Strings from separate elements are separately with '\n'
string Element::get_deep_content() const
{
  // Return content directly if available
  if (content != "") return content;

  // Otherwise iterate all elements
  string s;
  for(list<Element *>::const_iterator p=children.begin();
      p!=children.end();
      p++)
  {
    const Element &e=**p;

    // Extract content from this, recursively
    string ss=e.get_deep_content();
    if (!ss.empty())
    {
      s+=ss;
      s+='\n';  // Separate pieces with \n
    }
  }

  return s;
}

//--------------------------------------------------------------------------
// Get an attribute of the given name
// Returns attribute value
// Defaults to default value given (or "") if not present
// This exists to avoid modifying the attribute when using attrs["foo"]
// when foo doesn't exist (a completely stupid specification of [], IMHO)
string Element::get_attr(const string& attname, const string& def) const
{
  map<string,string>::const_iterator p=attrs.find(attname);
  if (p!=attrs.end())
    return p->second;
  else
    return def;
}

//--------------------------------------------------------------------------
// Get the boolean value of an attribute of the given name
// Returns attribute value
// Defaults to default value given (or false) if not present
// Recognises words beginning [TtYy] as true, everything else is false
bool Element::get_attr_bool(const string& attname, bool def) const
{
  map<string,string>::const_iterator p=attrs.find(attname);
  if (p!=attrs.end())
  {
    char c=0;
    if (!p->second.empty()) c=p->second[0];
      
    switch(c)
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
// Get the integer value of an attribute of the given name
// Returns attribute value
// Defaults to default value given (or 0) if not present
// Returns 0 if present but bogus
int Element::get_attr_int(const string& attname, int def) const
{
  map<string,string>::const_iterator p=attrs.find(attname);
  if (p!=attrs.end()) return atoi(p->second.c_str());

  return def;
}

//--------------------------------------------------------------------------
// Tests whether the element has an attribute of the given name
// Quicker than !get_attr("foo").empty()
bool Element::has_attr(const string& attname) const
{
  map<string,string>::const_iterator p=attrs.find(attname);
  if (p==attrs.end())
    return false;
  else
    return true;
}

//--------------------------------------------------------------------------
// Set an attribute (string)
void Element::set_attr(const string& attname, const string& value)
{
  attrs[attname] = value;
}

//--------------------------------------------------------------------------
// Set an attribute (integer)
// (_int qualifier not strictly necessary here, but matches get_attr_int)
void Element::set_attr_int(const string& attname, int value)
{
  ostringstream oss;
  oss << value;
  attrs[attname] = oss.str();
}

//--------------------------------------------------------------------------
// Set an attribute (bool)
// (_bool qualifier not strictly necessary here, but matches get_attr_bool)
void Element::set_attr_bool(const string& attname, bool value)
{
  attrs[attname] = value?"yes":"no";
}

//--------------------------------------------------------------------------
// Translate name using given map:
//   If not present, leave it and return true
//   If present but mapped to "", leave it return false (=> delete me)
//   If present and mapped to non empty, change to mapped string
// 
// Recurses to sub-elements and deletes them if they return false -
// net effect begin that names mapped to "" are (deep) deleted from
// the document
bool Element::translate(map<string, string>& trans_map)
{
  //Ignore (and keep) data elements
  if (name.empty()) return true;

  //Lookup current name in translation map
  map<string,string>::iterator tp=trans_map.find(name);

  //Check for map to empty first - no point in recursing if we're going
  //to be deleted anyway
  if (tp!=trans_map.end() && tp->second.empty())
    return false;  //Delete me

  //Recurse to sub-elements first
  for(list<Element *>::iterator p=children.begin();
      p!=children.end();
      )  //Incremented in body to avoid invalidity after erase
  {
    Element& se = **p;

    if (!se.translate(trans_map))
    {
      delete *p;
      p=children.erase(p);
    }
    else p++;
  }

  if (tp==trans_map.end()) return true;  // Leave me alone

  //We know it's not empty - change name
  name = tp->second;
  return true;  // Leave me alone now
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




