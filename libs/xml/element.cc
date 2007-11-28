//==========================================================================
// ObTools::XML: element.cc
//
// XML Element structure
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xml.h"
#include <ctype.h>
#include <sstream>
#include <algorithm>

using namespace ObTools::XML;

//--------------------------------------------------------------------------
// Non-element marker
Element Element::none("!NONE!");

//--------------------------------------------------------------------------
// Deep copy to a new element
// Copies the name, direct content and attributes into the given element
// and recursively copies children
// parent pointer of top element is not copied
void Element::deep_copy_to(Element& dest) const
{
  copy_to(dest);
  for(list<Element *>::const_iterator p=children.begin();
      p!=children.end();
      p++)
    dest.add((*p)->deep_copy());
}

//--------------------------------------------------------------------------
// Write attributes to given stream
void Element::write_attrs(ostream &s) const
{
  //Output attributes
  for(map<string,string>::const_iterator p=attrs.begin();
      p!=attrs.end();
      p++)
  {
    xmlchar delim='"';
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

    //But we must escape &, < and > as well
    s << ' ' << p->first << "=" << delim << escape(v, escquote) << delim;
  } 
}

//--------------------------------------------------------------------------
// Write to a given stream with given indent
void Element::write_indented(int indent, ostream &s) const
{
  for(int i=0; i<indent; i++) s<<' ';

  //Name indicates true element
  if (name.size())
  {
    // Output start tag and attributes
    s << '<' << name;
    write_attrs(s);

    //Output sub-elements if any
    if (children.empty())
    {
      //Could have an 'optimised' content string, though
      if (content.size())
      {
	//String it all on one line - escaped for &, < and >
	s << '>' << escape(content, false) << "</" << name << '>' << endl;
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
// with_pi controls whether to including the standard-compliant <?xml .. ?>
void Element::write_to(ostream &s, bool with_pi) const
{
  if (with_pi) s << "<?xml version=\"1.0\"?>" << endl;
  write_indented(0, s);
}

//--------------------------------------------------------------------------
// Convert to a string
// with_pi controls whether to including the standard-compliant <?xml .. ?>
string Element::to_string(bool with_pi) const
{
  ostringstream oss;
  write_to(oss, with_pi);
  return oss.str();
}

//--------------------------------------------------------------------------
// Write start-tag only to a given stream
// NB, always outputs unclosed start tag, even if empty
void Element::write_start_to(ostream &s) const
{
  s << "<" << name;
  write_attrs(s);
  s << ">";
}

//--------------------------------------------------------------------------
// Convert start-tag to a string
string Element::start_to_string() const
{
  ostringstream oss;
  write_start_to(oss);
  return oss.str();
}

//--------------------------------------------------------------------------
// Write end-tag only to a given stream
void Element::write_end_to(ostream &s) const
{
  s << "</" << name << ">";
}

//--------------------------------------------------------------------------
// Convert end-tag to a string
string Element::end_to_string() const
{
  ostringstream oss;
  write_end_to(oss);
  return oss.str();
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

//------------------------------------------------------------------------
// Add child elements from XML text - reparses text and adds resulting
// root as a child element.  Parser ostream & flags as Parser() below
// Returns added child, or 'none' if parse failed
Element& Element::add_xml(const string& xml, ostream& serr, int parse_flags)
{
  ObTools::XML::Parser parser(serr, parse_flags);
  try
  {
    parser.read_from(xml);
    Element *root = parser.detach_root();
    if (root)
    {
      children.push_back(root);
      return *root;
    }
    else return none;
  }
  catch (ParseFailed) 
  {
    return none;
  }
}

//--------------------------------------------------------------------------
// Find n'th (first, by default) child element, whatever it is
// Returns Element::none if none
// Const and non-const implementations
const Element& Element::get_child(int n) const
{
  list<Element *>::const_iterator p=children.begin();
  for(;n && p!=children.end(); ++p, --n)
    ;

  if (p!=children.end())
    return **p;
  else
    return Element::none;
}

Element& Element::get_child(int n)
{
  list<Element *>::iterator p=children.begin();
  for(;n && p!=children.end(); ++p, --n)
    ;

  if (p!=children.end())
    return **p;
  else
    return Element::none;
}

//--------------------------------------------------------------------------
// Find n'th (first, by default) child element of given name
// Returns Element::none if none
// Const and non-const implementations
const Element& Element::get_child(const string& ename, int n) const
{
  for(list<Element *>::const_iterator p=children.begin();
      p!=children.end();
      p++)
  {
    if ((*p)->name==ename && !n--) return **p;
  }
  return Element::none;
}

Element& Element::get_child(const string& ename, int n)
{
  for(list<Element *>::iterator p=children.begin();
      p!=children.end();
      p++)
  {
    if ((*p)->name==ename && !n--) return **p;
  }
  return Element::none;
}

//--------------------------------------------------------------------------
// Ensure the existence of a child of the given name, and return it
// Creates new child element of the given name if one doesn't already exist
Element& Element::make_child(const string& ename)
{
  Element& child = get_child(ename);
  if (!!child) return child;
  return add(ename);
}

//--------------------------------------------------------------------------
// Find first (or only) descendant of given name
// Returns Element::none if there isn't one 
// (Like get_child() but ignoring intervening cruft)
// Const and non-const implementations
const Element& Element::get_descendant(const string& ename) const
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

Element& Element::get_descendant(const string& ename)
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
// Recognises words beginning [TtYy1] as true, everything else is false
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
      case '1':
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
// Get the integer value of an attribute of the given name, from hex string
// Returns attribute value
// Defaults to default value given (or 0) if not present
// Returns 0 if present but bogus
int Element::get_attr_hex(const string& attname, int def) const
{
  map<string,string>::const_iterator p=attrs.find(attname);
  if (p!=attrs.end()) 
  {
    istringstream iss(p->second);
    iss >> hex >> def;
  }

  return def;
}

//--------------------------------------------------------------------------
// Get the 64-bit integer value of an attribute of the given name
// Returns attribute value
// Defaults to default value given (or 0) if not present
// Returns 0 if present but bogus
uint64_t Element::get_attr_int64(const string& attname, uint64_t def) const
{
  map<string,string>::const_iterator p=attrs.find(attname);

  if (p!=attrs.end())
  {
    // Note:  We can't use atoll here because it is signed and barfs on 
    // anything over 2^63-1
    sscanf(p->second.c_str(), "%llu", &def);
  }

  return def;
}

//--------------------------------------------------------------------------
// Get the 64-bit integer value of an attribute of the given name, from hex
// string
// Returns attribute value
// Defaults to default value given (or 0) if not present
// Returns 0 if present but bogus
uint64_t Element::get_attr_hex64(const string& attname, uint64_t def) const
{
  map<string,string>::const_iterator p=attrs.find(attname);

  if (p!=attrs.end()) 
  {
    istringstream iss(p->second);
    iss >> hex >> def;
  }

  return def;
}


//--------------------------------------------------------------------------
// Get the real value of an attribute of the given name
// Returns attribute value
// Defaults to default value given (or 0.0) if not present
// Returns 0.0 if present but bogus
double Element::get_attr_real(const string& attname, double def) const
{
  map<string,string>::const_iterator p=attrs.find(attname);
  if (p!=attrs.end()) return atof(p->second.c_str());

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
Element& Element::set_attr(const string& attname, const string& value)
{
  attrs[attname] = value;
  return *this;
}

//--------------------------------------------------------------------------
// Set an attribute (integer)
// (_int qualifier not strictly necessary here, but matches get_attr_int)
Element& Element::set_attr_int(const string& attname, int value)
{
  ostringstream oss;
  oss << value;
  attrs[attname] = oss.str();
  return *this;
}

//--------------------------------------------------------------------------
// Set an attribute (integer, hex)
// (_int qualifier not strictly necessary here, but matches get_attr_int)
Element& Element::set_attr_hex(const string& attname, int value)
{
  ostringstream oss;
  oss << hex << value;
  attrs[attname] = oss.str();
  return *this;
}

//--------------------------------------------------------------------------
// Set an attribute (64-bit integer)
// (_int64 qualifier not strictly necessary here, but matches get_attr_int64)
Element& Element::set_attr_int64(const string& attname, uint64_t value)
{
  ostringstream oss;
  oss << value;
  attrs[attname] = oss.str();
  return *this;
}

//--------------------------------------------------------------------------
// Set an attribute (64-bit integer, hex)
// (_int64 qualifier not strictly necessary here, but matches get_attr_int64)
Element& Element::set_attr_hex64(const string& attname, uint64_t value)
{
  ostringstream oss;
  oss << hex << value;
  attrs[attname] = oss.str();
  return *this;
}

//--------------------------------------------------------------------------
// Set an attribute (bool)
// (_bool qualifier not strictly necessary here, but matches get_attr_bool)
Element& Element::set_attr_bool(const string& attname, bool value)
{
  attrs[attname] = value?"true":"false";
  return *this;
}

//--------------------------------------------------------------------------
// Set an attribute (real)
// (_real qualifier not strictly necessary here, but matches get_attr_real)
Element& Element::set_attr_real(const string& attname, double value)
{
  ostringstream oss;
  oss << value;
  attrs[attname] = oss.str();
  return *this;
}

//--------------------------------------------------------------------------
// Get XPath position relative to root (not including the root itself)
// Returns an XPath string that can be used to identify this element in
// the same document
string Element::get_xpath() const
{
  string xpath;

  // Loop up through parents, stop at root
  for(const Element *current=this; 
      current && current->parent; 
      current=current->parent)
  {
    ostringstream oss;
    oss << "/" << current->name;

    // Check where we are in the list of siblings with the same name
    int count=0;
    int mycount=0;
    list<Element *>& sibs = current->parent->children;

    for(list<Element *>::const_iterator p=sibs.begin();
	p!=sibs.end();
	++p)
    {
      if (*p == current) mycount = count;
      if ((*p)->name == current->name) count++;
    }

    // Add count part if required
    if (count>1) oss << "[" << mycount+1 << "]";

    // Add this to the front
    xpath.insert(0, oss.str());
  }

  return xpath;
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
// Detach from parent
void Element::detach()
{
  if (parent) parent->children.remove(this);
  parent = 0;
}

//--------------------------------------------------------------------------
// Replace with the given element at same position in parent
// Detaches this element and attaches the new one
void Element::replace_with(Element *e)
{
  if (!parent) return;

  // Find current position in parent
  list<Element *>& l = parent->children;
  list<Element *>::iterator p = find(l.begin(), l.end(), this);

  // If there, swap it with 'e' and detach
  if (p!=l.end())
  {
    l.insert(p, e);
    e->parent = parent;

    l.erase(p);
    parent = 0;
  }
}

//--------------------------------------------------------------------------
// Clear children
// Recursively destroys children
void Element::clear_children()
{
  for(list<Element *>::iterator p=children.begin();
      p!=children.end();
      p++)
    delete *p;
  children.clear();
}

//--------------------------------------------------------------------------
// Destructor
// Recursively destroys children
Element::~Element()
{
  clear_children();
}

//------------------------------------------------------------------------
// >> operator to write to ostream
ostream& ObTools::XML::operator<<(ostream& s, const Element& e) 
{
  e.write_to(s);
  return s;
}




