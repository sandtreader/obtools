//==========================================================================
// ObTools::XML: ot-xml.h
//
// Public definitions for ObTools XML parser
// 
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XML_H
#define __OBTOOLS_XML_H

#include <string>
#include <deque>
#include <list>
#include <map>
#include <iostream>

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
			  list<Element *>& l);

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

  //------------------------------------------------------------------------
  // Constructors & Destructors
  Element(string n):name(n) { }
  Element(string n, string c):name(n), content(c) {  }
  ~Element();
 
  //------------------------------------------------------------------------
  // Dump to given input stream
  void write_to(ostream& s) const; 

  //------------------------------------------------------------------------
  // 'Optimise' single text sub-elements back to 'content' string here
  void optimise(); 

  //--------------------------------------------------------------------------
  // Find first (or only) child element of given name
  // Returns Element::none if there isn't one 
  //   Use valid() to check which you've got - e.g.:
  //     XML::Element& e = get_child(root, "foo");
  //     if (e.valid())
  //       //Use e
  Element &get_child(const string& name);

  //--------------------------------------------------------------------------
  // Find all child elements of given name
  // Returns list of pointers
  list<Element *> get_children(const string& name);

  //--------------------------------------------------------------------------
  // Find all descendant elements of given name - recursive
  // Returns flat list of pointers
  // Prunes tree walk at 'prune' tags if set - use for recursive structures
  // where you want to deal with each level independently
  // Name and prune can be the same - then returns only first level of 
  // <name>s, not <name>s within <name>s
  list<Element *> get_descendants(const string& name,
				  const string& prune="");

  //--------------------------------------------------------------------------
  // Get an attribute of the given name
  // Returns attribute value
  // Defaults to default value given (or "") if not present
  // This exists to avoid creating the attribute when using attrs["foo"]
  // when foo doesn't exist (a completely stupid specification of [], IMHO)
  string get_attr(const string& name, const char *def="");

  //--------------------------------------------------------------------------
  // Get the boolean value of an attribute of the given name
  // Returns attribute value
  // Defaults to default value given (or false) if not present
  // Recognises words beginning [TtYy] as true, everything else is false
  bool get_attr_bool(const string& name, bool def=false);

  //--------------------------------------------------------------------------
  // Get the integer value of an attribute of the given name
  // Returns attribute value
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  int get_attr_int(const string& name, int def=0);

  //--------------------------------------------------------------------------
  // Tests whether the element has an attribute of the given name
  // Quicker than !get_attr("foo").empty()
  bool has_attr(const string& name);

  //--------------------------------------------------------------------------
  // Get all direct child text content accumulated into one string
  // Returns optimised content if available, otherwise iterates children
  // collecting text from data elements
  // Strings from separate elements are separately with '\n'
  string get_content();

  //--------------------------------------------------------------------------
  // Get all text content from the entire tree accumulated into one string
  // Returns optimised content if available, otherwise iterates children
  // collecting text from data elements, and recursing into subchildren
  // Strings from separate elements are separately with '\n'
  string get_deep_content();

  //--------------------------------------------------------------------------
  // Validity check
  // Easy way of checking if you've got Element::none
  // Don't use (e!=Element::none) - that compares values!
  bool valid() { return this!=&none; }
};

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
  // Get root element
  // Throws ParseFailed if not valid
  Element& get_root() throw (ParseFailed); 
};

//==========================================================================
// XML syntactic support macros (inlines won't do this, sorry)
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
// XML stream operators

//------------------------------------------------------------------------
// >> operator to parse from istream
//
// e.g. cin >> parser;
//
// Throws ParseFailed if bad XML received
istream& operator>>(istream& s, Parser& p) throw (ParseFailed);

//------------------------------------------------------------------------
// >> operator to write Element to ostream
//
// e.g. cout << parser.get_root();
//
// Note - read into Parser, write from Element
ostream& operator<<(ostream& s, const Element& e);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XML_H



