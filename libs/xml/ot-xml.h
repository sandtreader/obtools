//==========================================================================
// ObTools::XML: ot-xml.h
//
// Public definitions for ObTools XML parser
// 
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_XML_H
#define __OBTOOLS_XML_H

#include <string>
#include <deque>
#include <list>
#include <map>
#include <iostream>
#include <stdint.h>

namespace ObTools { namespace XML {

//Make our lives easier without polluting anyone else
using namespace std;

//Type to use for characters
typedef char xmlchar;

//==========================================================================
// XML Element class
// XML document is a tree of these
class Element
{
private:
  void write_indented(int indent, ostream &s) const;
  string escape(const string &v, bool escquote) const;
  void append_descendants(const string& name, const string& prune, 
			  list<const Element *>& l) const;
  void append_descendants(const string& name, const string& prune, 
			  list<Element *>& l);

  //Prevent copy and assignment - don't want to do deep copy
  Element(const Element& e) {}
  const Element& operator=(const Element& e) {}

public:
  //Element name ('tag') - empty for data 'elements'
  string name;

  //Element content - set only for data 'elements', or if 'OPTIMISE'
  //flag set, when single data elements are snapped back to the content
  //of the parent
  string content;

  //Attributes attr-name -> attr-value
  //BEWARE of using [] to access this - will modify map and add empty
  //attributes if it doesn't exist.  Use get_attr() in preference if
  //you need to restream the input
  map<string,string> attrs;

  //List of sub-elements
  list<Element *> children;

  //--------------------------------------------------------------------------
  // Non-element marker
  static Element none;

  //--------------------------------------------------------------------------
  // Validity checks
  // Easy way of checking if you've got Element::none
  // Don't use (e!=Element::none) - that compares values!
  bool valid() const { return this!=&none; }
  bool operator!() const { return !valid(); }

  //------------------------------------------------------------------------
  // Constructors 
  // One string: just the name
  Element(const string& n):name(n) { }

  // Two strings: name and textual content
  Element(const string& n, const string& c):name(n), content(c) {  }

  // Three strings: name and one attribute - e.g. namespace
  Element(const string& n, const string& a, const string& v): name(n) 
    { set_attr(a,v); }

  // Four strings: name, one attribute and content
  Element(const string& n, const string& a, const string& v, const string& c):
    name(n), content(c) { set_attr(a,v); }

  //------------------------------------------------------------------------
  // Add a child element
  Element& add(Element *child) { children.push_back(child); return *child; }
 
  //------------------------------------------------------------------------
  // Add a new empty child element by name
  // Also 2-, 3- and 4-string options (see constructors above)
  Element& add(const string& n) { return add(new Element(n)); }

  Element& add(const string& n, const string& c) 
  { return add(new Element(n, c)); }

  Element& add(const string& n, const string& a, const string& v) 
  { return add(new Element(n, a, v)); }

  Element& add(const string& n, const string& a, 
	       const string& v, const string& c) 
  { return add(new Element(n, a, v, c)); }

  //------------------------------------------------------------------------
  // Dump to given output stream
  void write_to(ostream& s) const; 

  //--------------------------------------------------------------------------
  // Convert to a string
  string to_string() const;

  //------------------------------------------------------------------------
  // 'Optimise' single text sub-elements back to 'content' string here
  void optimise(); 

  //--------------------------------------------------------------------------
  // Find first (or only) child element, whatever it is
  // Returns Element::none if there isn't one 
  // Const and non-const implementations (likewise rest of accessors)
  const Element& get_child() const;
  Element& get_child();

  //--------------------------------------------------------------------------
  // Find first (or only) child element of given name
  // Returns Element::none if there isn't one 
  //   Use valid() to check which you've got - e.g.:
  //     XML::Element& e = get_child(root, "foo");
  //     if (e.valid())
  //       //Use e
  const Element& get_child(const string& ename) const;
  Element& get_child(const string& ename);

  //--------------------------------------------------------------------------
  // Ensure the existence of a child of the given name, and return it
  // Creates new child element of the given name if one doesn't already exist
  Element& make_child(const string& ename);

  //--------------------------------------------------------------------------
  // Find first (or only) descendant of given name
  // Returns Element::none if there isn't one 
  // (Like get_child() but ignoring intervening cruft)
  const Element& get_descendant(const string& ename) const;
  Element& get_descendant(const string& ename);

  //--------------------------------------------------------------------------
  // Find all child elements of given name
  // Returns list of pointers
  list<const Element *> get_children(const string& ename) const;
  list<Element *> get_children(const string& ename);

  //--------------------------------------------------------------------------
  // Find all descendant elements of given name - recursive
  // Returns flat list of pointers
  // Prunes tree walk at 'prune' tags if set - use for recursive structures
  // where you want to deal with each level independently
  // Ename and prune can be the same - then returns only first level of 
  // <ename>s, not <ename>s within <ename>s
  list<const Element *> get_descendants(const string& ename,
					const string& prune="") const;
  list<Element *> get_descendants(const string& ename,
				  const string& prune="");

  //--------------------------------------------------------------------------
  // Get an attribute of the given name
  // Returns attribute value
  // Defaults to default value given (or "") if not present
  // This exists to avoid creating the attribute when using e.attrs["foo"]
  // when foo doesn't exist - use e["foo"] instead
  string get_attr(const string& attname, const string& def="") const;

  //--------------------------------------------------------------------------
  // Handy [] operator to get attribute values
  string operator[](const string& attr) const { return get_attr(attr); }

  //--------------------------------------------------------------------------
  // Get the boolean value of an attribute of the given name
  // Returns attribute value
  // Defaults to default value given (or false) if not present
  // Recognises words beginning [TtYy1] as true, everything else is false
  bool get_attr_bool(const string& attname, bool def=false) const;

  //--------------------------------------------------------------------------
  // Get the integer value of an attribute of the given name
  // Returns attribute value
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  int get_attr_int(const string& attname, int def=0) const;

  //--------------------------------------------------------------------------
  // Get the 64-bit integer value of an attribute of the given name
  // Returns attribute value
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  uint64_t get_attr_int64(const string& attname, uint64_t def=0) const;

  //--------------------------------------------------------------------------
  // Tests whether the element has an attribute of the given name
  // Quicker than !get_attr("foo").empty()
  bool has_attr(const string& attname) const;

  //--------------------------------------------------------------------------
  // Set an attribute (string)
  void set_attr(const string& attname, const string& value);

  //--------------------------------------------------------------------------
  // Set an attribute (integer)
  // (_int qualifier not strictly necessary here, but matches get_attr_int)
  void set_attr_int(const string& attname, int value);

  //--------------------------------------------------------------------------
  // Set an attribute (bool)
  // (_bool qualifier not strictly necessary here, but matches get_attr_bool)
  void set_attr_bool(const string& attname, bool value);

  //--------------------------------------------------------------------------
  // Get all direct child text content accumulated into one string
  // Returns optimised content if available, otherwise iterates children
  // collecting text from data elements
  // Strings from separate elements are separately with '\n'
  string get_content() const;

  //--------------------------------------------------------------------------
  // Get all text content from the entire tree accumulated into one string
  // Returns optimised content if available, otherwise iterates children
  // collecting text from data elements, and recursing into subchildren
  // Strings from separate elements are separately with '\n'
  string get_deep_content() const;

  //--------------------------------------------------------------------------
  // Translate name using given map:
  //   If not present, leave it and return true 
  //   If present but mapped to "", leave it & return false (=> delete me)
  //   If present and mapped to non empty, change to mapped string
  // 
  // Recurses to sub-elements and deletes them if they return false -
  // net effect being that names mapped to "" are (deep) deleted from
  // the document.  
  bool translate(map<string, string>& trans_map);

  //------------------------------------------------------------------------
  // Destructor
  ~Element();
};

//==========================================================================
// XML syntactic support macros (inline template functions won't do this, 
// sorry Bjarne ;-)
//
// e.g.
//    OBTOOLS_XML_FOREACH_CHILD_TAG(s, root, "section")
//      cout << s.get_attr("name");
//    OBTOOLS_XML_ENDFOR

// Do block for every direct child of a parent
// (Read as 'for each child <v> of parent <p>')
#define OBTOOLS_XML_FOREACH_CHILD(_childvar, _parent)                     \
  {                                                                       \
    list<ObTools::XML::Element *>& _elems=(_parent).children;             \
    for(list<ObTools::XML::Element *>::const_iterator _p=_elems.begin();  \
        _p!=_elems.end();                                                 \
        _p++)                                                             \
    {                                                                     \
      ObTools::XML::Element& _childvar=**_p;

// Do block for every direct child with a given tag
// (Read as 'for each child <v> of parent <p> with tag <tag>')
#define OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(_childvar, _parent, _tag)      \
  {                                                                       \
    list<ObTools::XML::Element *> _elems=(_parent).get_children(_tag);    \
    for(list<ObTools::XML::Element *>::const_iterator _p=_elems.begin();  \
        _p!=_elems.end();                                                 \
        _p++)                                                             \
    {                                                                     \
      ObTools::XML::Element& _childvar=**_p;

// Do block for every descendant with a given tag
// (Read as 'for each descendant <v> of parent <p> with tag <tag>')
#define OBTOOLS_XML_FOREACH_DESCENDANT_WITH_TAG(_childvar, _parent, _tag) \
  {                                                                       \
    list<ObTools::XML::Element *> _elems=(_parent).get_descendants(_tag); \
    for(list<ObTools::XML::Element *>::const_iterator _p=_elems.begin();  \
        _p!=_elems.end();                                                 \
        _p++)                                                             \
    {                                                                     \
      ObTools::XML::Element& _childvar=**_p;

// Do block for every descendant with a given tag, but pruned at given tag
// (Read as 'for each descendant <v> of parent <p> with tag <tag>, 
//  pruned at <prune>s')
#define OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(_childvar, _parent,\
                                                        _tag, _prune)     \
  {                                                                       \
    list<ObTools::XML::Element *> _elems=                                 \
       (_parent).get_descendants(_tag, _prune);                           \
    for(list<ObTools::XML::Element *>::const_iterator _p=_elems.begin();  \
        _p!=_elems.end();                                                 \
        _p++)                                                             \
    {                                                                     \
      ObTools::XML::Element& _childvar=**_p;

 
// End block for any kind of FOREACH
#define OBTOOLS_XML_ENDFOR }}

//==========================================================================
// XML exceptions
class ParseFailed {};

//==========================================================================
// XML Parser flags
enum 
{
  // Snap single text content elements back to parent-element.content
  // Makes simple grammars a lot easier to access
  PARSER_OPTIMISE_CONTENT=1,

  // Preserve whitespace as it (prepare for a flood of indentation strings!)
  // BEWARE: If you use this, the << >> process isn't round-trip clean for
  // whitespace, because the output adds its own indentation
  PARSER_PRESERVE_WHITESPACE=2,

  // Do namespace translation
  // Automatically turned on if you call fix_namespace
  PARSER_FIX_NAMESPACES=4
};

//==========================================================================
// XML Parser class
// Holds global parsing configuration
class Parser
{
private:
  //Persistent state
  ostream& serr;       //error output stream
  int flags;
  map<string, string> user_ns_map;  //Map of full name->prefix

  //Transient per-document state
  deque<Element *> elements;  //Could be a <stack>, but no clear()
  Element *root;       //0 if not valid
  deque<map<string, string> > ns_maps; //Stack of maps of prefix->full name

  void parse_stream(istream &s) throw (ParseFailed);
  void read_tag(xmlchar c, istream &s) throw(ParseFailed);
  void read_end_tag(xmlchar c, istream &s) throw(ParseFailed);
  void read_content(xmlchar c, istream &s) throw(ParseFailed);
  void read_ref(string& text, istream &s) throw (ParseFailed);
  string read_rest_of_name(xmlchar& c, istream& s);
  void skip_comment(istream &s) throw (ParseFailed);
  void skip_to_gt(istream &s) throw (ParseFailed);
  void skip_pi(istream &s) throw (ParseFailed);
  void error(const char *s);
  void fatal(const char *s) throw (ParseFailed);
  void initial_processing(Element *e);
  void final_processing(Element *e);
  void substitute_name(string& name, bool usedef=false);

public:
  int errors;
  
  //------------------------------------------------------------------------
  // Constructors & Destructor
  // s is output stream for parsing errors
  Parser(ostream &s, int f = PARSER_OPTIMISE_CONTENT):
    serr(s), 
    errors(0),
    root(0),
    flags(f) 
  {}

  // Default - use cerr
  Parser(int f = PARSER_OPTIMISE_CONTENT):
    serr(cerr), 
    errors(0),
    root(0),
    flags(f)
  {}

  ~Parser();

  //------------------------------------------------------------------------
  // Add namespace to prefix mapping
  // Any prefix used for this namespace in the document will be replaced
  // for the given prefix.  This allows you to normalise the tags you
  // check for
  void fix_namespace(const char *name, const char *prefix);

  //------------------------------------------------------------------------
  // Parse from given input stream
  // Throws ParseFailed if parse fails for any fatal reason
  // See also istream operator >> below, which is nicer
  void read_from(istream& s) throw (ParseFailed); 

  //------------------------------------------------------------------------
  // Parse from given string
  // Throws ParseFailed if parse fails for any fatal reason
  void read_from(const string& s) throw (ParseFailed); 

  //------------------------------------------------------------------------
  // Get root element
  // Returns Element::none if not valid
  Element& get_root(); 

  //------------------------------------------------------------------------
  // Get root element detached from parser, so you can keep it after the
  // parser has died
  // Returns 0 if not valid
  Element *detach_root(); 
};


//==========================================================================
// XML stream operators

//------------------------------------------------------------------------
// >> operator to parse from istream
//
// e.g. cin >> parser;
//
// Throws ParseFailed if bad XML received
istream& operator>>(istream& s, Parser& p) throw (ParseFailed);

//------------------------------------------------------------------------
// << operator to write Element to ostream
//
// e.g. cout << parser.get_root();
//
// Note - read into Parser, write from Element
ostream& operator<<(ostream& s, const Element& e);

//==========================================================================
// XPath processor
// Only handles child and attribute axis steps in abbreviated form, no 
// predicates
// e.g. /config/foo/@width
// Paths can be absolute or relative - always rooted at 'root'

class XPathProcessor
{
private:
  Element& root;           // Document root we're working on

public:
  //------------------------------------------------------------------------
  // Constructors
  XPathProcessor(): root(Element::none) {}
  XPathProcessor(Element& _root): root(_root) {}
  
  //------------------------------------------------------------------------
  // Element list fetch - all elements matching final child step.
  // Only first element of intermediate steps is used - list is not merged!
  list<Element *> get_elements(const string& path);

  //------------------------------------------------------------------------
  // Single element fetch - first of list, if any, or 0
  Element *get_element(const string& path);

  //------------------------------------------------------------------------
  // Value fetch - either attribute or content of single (first) element
  // Returns def if anything not found
  string get_value(const string& path, const string& def="");

  //------------------------------------------------------------------------
  // [] operator to make things easy
  // e.g. xpath["foo/bar"]
  string operator[](const string& path) { return get_value(path); }

  //--------------------------------------------------------------------------
  // Boolean value fetch
  // Defaults to default value given (or false) if not present
  // Recognises words beginning [TtYy] as true, everything else is false
  bool get_value_bool(const string& path, bool def=false);

  //--------------------------------------------------------------------------
  // Integer value fetch
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  int get_value_int(const string& path, int def=0);
};

//==========================================================================
// Configuration
// Support for XML configuration files
class Configuration
{
private:
  list<string> filenames;
  Parser parser;

public:
  //------------------------------------------------------------------------
  // Constructors
  // Single filename
  Configuration(const string& fn, int parse_flags=PARSER_OPTIMISE_CONTENT)
    :parser(parse_flags) { filenames.push_back(fn); }

  // List of filenames - front() is tried first
  Configuration(list<string>& fns, int parse_flags=PARSER_OPTIMISE_CONTENT)
    :filenames(fns), parser(parse_flags) {}

  //------------------------------------------------------------------------
  // Read configuration file
  // Returns whether successful
  // ename is the expected root element name - fails if wrong
  bool read(const string& ename);

  //------------------------------------------------------------------------
  // Get root element
  // Returns Element::none if not valid
  Element& get_root() { return parser.get_root(); }

  //------------------------------------------------------------------------
  // Element list fetch - all elements matching final child step.
  // Only first element of intermediate steps is used - list is not merged!
  list<Element *> get_elements(const string& path);

  //------------------------------------------------------------------------
  // Single element fetch - first of list, if any, or 0
  Element *get_element(const string& path);

  //------------------------------------------------------------------------
  // XPath value fetch - either attribute or content of single (first) element
  // Returns def if anything not found
  // Note all get_value methods still work, and return def, if file read fails
  string get_value(const string& path, const string& def="");

  //------------------------------------------------------------------------
  // [] operator to make things easy
  // e.g. config["foo/bar"]
  string operator[](const string& path) { return get_value(path); }

  //--------------------------------------------------------------------------
  // XPath Boolean value fetch
  // Defaults to default value given (or false) if not present
  // Recognises words beginning [TtYy] as true, everything else is false
  bool get_value_bool(const string& path, bool def=false);

  //--------------------------------------------------------------------------
  // XPath Integer value fetch
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  int get_value_int(const string& path, int def=0);

  //------------------------------------------------------------------------
  // XPath list-of-values fetch
  // Returns contents of all elements matching XPath
  list<string> get_values(const string& path);

  //------------------------------------------------------------------------
  // XPath map fetch
  // Returns string->string map of all element matching given XPath,
  // using given attribute name as key, content as value
  map<string, string> get_map(const string& path,
			      const char *name_attr = "name");
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XML_H



