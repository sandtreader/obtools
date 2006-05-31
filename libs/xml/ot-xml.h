//==========================================================================
// ObTools::XML: ot-xml.h
//
// Public definitions for ObTools XML parser
// 
// Copyright (c) 2003-2005 xMill Consulting Limited.  All rights reserved
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

class Element; // Forward

//==========================================================================
// Element list iterators
// Actually contains a list of elements;  sequential forward access only
// Usage is different from STL iterator:
//    for(Element::iterator p(parent.children()); p; ++p)
//       XML::Element& child = *p;
struct ElementIterator
{
  list<Element *> elements;
  list<Element *>::const_iterator it;

  //------------------------------------------------------------------------
  // Constructors
  ElementIterator(const list<Element *>& l): 
    elements(l), it(elements.begin()) {}
  ElementIterator(const ElementIterator& o):
    elements(o.elements), it(elements.begin()) {}

  //------------------------------------------------------------------------
  // Validity checks
  bool valid() { return it != elements.end(); }
  operator bool() { return valid(); }
  bool operator!() { return !valid(); }

  //------------------------------------------------------------------------
  // Incrementors
  // Postincrement (p++) is not provided because it would be very inefficient
  ElementIterator& operator++() { it++; return *this; }

  //------------------------------------------------------------------------
  // Derefs
  Element& operator*() const { return **it; }
  Element *operator->() const { return *it; }
};

// Const equivalent
struct ConstElementIterator
{
  list<const Element *> elements;
  list<const Element *>::const_iterator it;
  ConstElementIterator(const list<const Element *>& l): 
    elements(l), it(elements.begin()) {}
  ConstElementIterator(const ConstElementIterator& o):
    elements(o.elements), it(elements.begin()) {}
  bool valid() { return it != elements.end(); }
  operator bool() { return valid(); }
  bool operator!() { return !valid(); }
  ConstElementIterator& operator++() { it++; return *this; }
  const Element& operator*() const { return **it; }
  const Element *operator->() const { return *it; }
};

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
  PARSER_FIX_NAMESPACES=4,

  // Be lenient about & and < in contexts in which they couldn't be XML
  // syntax - i.e., not followed by a name character, '#' (for &), 
  // '!', '?' or '/' (for <) - and consider them just normal character data.
  // This makes it more pleasant to include code, particularly C++ code, 
  // in XML

  // This mimics the behaviour of SGML (hooray!) (ISO 8879:B.7.3), but
  // strictly violates XML (XML1.0:2.4), unless you consider it a form
  // of error handling...  But seriously, folks, using this feature may
  // mean your XML files are rejected by parsers who are less lenient
  // (or by this parser without this flag set)
  PARSER_BE_LENIENT=8
};

//==========================================================================
// XML Element class
// XML document is a tree of these
class Element
{
private:
  void write_attrs(ostream &s) const;
  void write_indented(int indent, ostream &s) const;
  string escape(const string &v, bool escquote) const;
  void append_descendants(const string& name, const string& prune, 
			  list<const Element *>& l) const;
  void append_descendants(const string& name, const string& prune, 
			  list<Element *>& l);

  //Prevent copy and assignment - don't want to do deep copy
  Element(const Element&) {}
  const Element& operator=(const Element& e) { return e; }

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

  //Parent element
  Element *parent;

  //Error support
  int line;   // Line number of our start tag

  //--------------------------------------------------------------------------
  // Non-element marker
  static Element none;

  //--------------------------------------------------------------------------
  // Iterator defs
  typedef ElementIterator iterator;
  typedef ConstElementIterator const_iterator;

  //--------------------------------------------------------------------------
  // Validity checks
  // Easy way of checking if you've got Element::none
  // Don't use (e!=Element::none) - that compares values!
  // Note we don't provide an operator bool due to problems with ambiguity
  // of [] operator
  bool valid() const { return this!=&none; }
  bool operator!() const { return !valid(); }

  //------------------------------------------------------------------------
  // Constructors 
  // Default - no name, no content, for later filling in
  Element():parent(0), line(0) { }

  // One string: just the name
  Element(const string& n):name(n), parent(0), line(0) { }

  // Two strings: name and textual content
  Element(const string& n, const string& c):
    name(n), content(c), parent(0), line(0) {  }

  // Three strings: name and one attribute - e.g. namespace
  Element(const string& n, const string& a, const string& v): 
    name(n), parent(0), line(0){ set_attr(a,v); }

  // Four strings: name, one attribute and content
  Element(const string& n, const string& a, const string& v, const string& c):
    name(n), content(c), parent(0), line(0) { set_attr(a,v); }

  //------------------------------------------------------------------------
  // Add a child element
  Element& add(Element *child) 
  { children.push_back(child); child->parent = this; return *child; }
 
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
  // Add child elements from XML text - reparses text and adds resulting
  // root as a child element.  Parser ostream & flags as Parser() below
  // Returns added child, or 'none' if parse failed
  Element& add_xml(const string& xml, ostream& serr = cerr,
		   int parse_flags = PARSER_OPTIMISE_CONTENT);

  //------------------------------------------------------------------------
  // Dump to given output stream
  // with_pi controls whether to including the standard-compliant <?xml .. ?>
  void write_to(ostream& s, bool with_pi=false) const; 

  //--------------------------------------------------------------------------
  // Convert to a string
  // with_pi controls whether to including the standard-compliant <?xml .. ?>
  string to_string(bool with_pi=false) const;

  //--------------------------------------------------------------------------
  // Write start-tag only to a given stream
  // NB, always outputs unclosed start tag, even if empty
  void write_start_to(ostream &s) const;

  //--------------------------------------------------------------------------
  // Convert start-tag to a string
  string start_to_string() const;

  //--------------------------------------------------------------------------
  // Write end-tag only to a given stream
  void write_end_to(ostream &s) const;

  //--------------------------------------------------------------------------
  // Convert end-tag to a string
  string end_to_string() const;

  //------------------------------------------------------------------------
  // 'Optimise' single text sub-elements back to 'content' string here
  void optimise(); 

  //--------------------------------------------------------------------------
  // Find n'th (first, by default) child element, whatever it is
  // Returns Element::none if there isn't one 
  // Const and non-const implementations (likewise rest of accessors)
  const Element& get_child(int n=0) const;
  Element& get_child(int n=0);

  //--------------------------------------------------------------------------
  // Find n'th (first, by default) child element of given name
  // Returns Element::none if there isn't one 
  //   Use valid() to check which you've got - e.g.:
  //     XML::Element& e = get_child(root, "foo");
  //     if (e.valid())
  //       //Use e
  const Element& get_child(const string& ename, int n=0) const;
  Element& get_child(const string& ename, int n=0);

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
  // Get the real value of an attribute of the given name
  // Returns attribute value
  // Defaults to default value given (or 0.0) if not present
  // Returns 0.0 if present but bogus
  double get_attr_real(const string& attname, double def=0.0) const;

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
  // Set an attribute (64-bit integer)
  // (_int64 qualifier not strictly necessary here, but matches get_attr_int64)
  void set_attr_int64(const string& attname, uint64_t value);

  //--------------------------------------------------------------------------
  // Set an attribute (bool)
  // (_bool qualifier not strictly necessary here, but matches get_attr_bool)
  void set_attr_bool(const string& attname, bool value);

  //--------------------------------------------------------------------------
  // Set an attribute (real)
  // (_real qualifier not strictly necessary here, but matches get_attr_real)
  void set_attr_real(const string& attname, double value);

  //--------------------------------------------------------------------------
  // Get all direct child text content accumulated into one string
  // Returns optimised content if available, otherwise iterates children
  // collecting text from data elements
  // Strings from separate elements are separately with '\n'
  string get_content() const;

  //--------------------------------------------------------------------------
  // Handy * operator to get content
  string operator*() const { return get_content(); }

  //--------------------------------------------------------------------------
  // Get all text content from the entire tree accumulated into one string
  // Returns optimised content if available, otherwise iterates children
  // collecting text from data elements, and recursing into subchildren
  // Strings from separate elements are separately with '\n'
  string get_deep_content() const;

  //--------------------------------------------------------------------------
  // Get XPath position relative to root (not including the root itself)
  // Returns an XPath string that can be used to identify this element in
  // the same document
  string get_xpath() const;

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

  //--------------------------------------------------------------------------
  // Detach from parent
  void detach();

  //--------------------------------------------------------------------------
  // Replace with the given element at same position in parent
  // Detaches this element and attaches the new one
  void replace_with(Element *e);

  //--------------------------------------------------------------------------
  // Clear children
  // Recursively destroys children
  void clear_children();

  //------------------------------------------------------------------------
  // Destructor
  ~Element();
};

//==========================================================================
// XML syntactic support macros 

// DEPRECATED:  Use ElementIterators directly in new code, and convert old!
// NOTE: Const versions of these are not provided - use const_iterators
//
// e.g.
//    OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(s, root, "section")
//      cout << s.get_attr("name");
//    OBTOOLS_XML_ENDFOR

// Do block for every direct child of a parent
// (Read as 'for each child <v> of parent <p>')
#define OBTOOLS_XML_FOREACH_CHILD(_childvar, _parent)                     \
  for(ObTools::XML::ElementIterator _p((_parent).children); _p; ++_p)   \
  { ObTools::XML::Element& _childvar=*_p;

// Do block for every direct child with a given tag
// (Read as 'for each child <v> of parent <p> with tag <tag>')
#define OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(_childvar, _parent, _tag)      \
  for(ObTools::XML::ElementIterator _p((_parent).get_children(_tag));     \
      _p; ++_p)                                                         \
  { ObTools::XML::Element& _childvar=*_p;

// Do block for every descendant with a given tag
// (Read as 'for each descendant <v> of parent <p> with tag <tag>')
#define OBTOOLS_XML_FOREACH_DESCENDANT_WITH_TAG(_childvar, _parent, _tag) \
  for(ObTools::XML::ElementIterator _p((_parent).get_descendants(_tag));  \
      _p; ++_p)                                                         \
  { ObTools::XML::Element& _childvar=*_p;

// Do block for every descendant with a given tag, but pruned at given tag
// (Read as 'for each descendant <v> of parent <p> with tag <tag>, 
//  pruned at <prune>s')
#define OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(_childvar, _parent,\
                                                        _tag, _prune)     \
  for(ObTools::XML::ElementIterator                                       \
        _p((_parent).get_descendants(_tag, _prune)); _p; ++_p)            \
  { ObTools::XML::Element& _childvar=*_p;

// --- End block for any kind of FOREACH
#define OBTOOLS_XML_ENDFOR }

//==========================================================================
// XML exceptions
class ParseFailed {};

//==========================================================================
// XML Parser class
// Holds global parsing configuration
class Parser
{
private:
  //Persistent state
  map<string, string> user_ns_map;  //Map of full name->prefix

  //Transient per-document state
  deque<Element *> elements;  //Could be a <stack>, but no clear()
  Element *root;       //0 if not valid
  deque<map<string, string> > ns_maps; //Stack of maps of prefix->full name

  //--------------------------------------------------------------------------
  //Inline character classification functions 
  //I18N: Somewhat dangerously, we use standard isalnum - which uses the
  //standard locale.  Behaviour of this outside ASCII is not known!
  bool is_name_start(xmlchar c)
  { return (isalnum(c) || c==':' || c=='_'); }
  bool is_name_char(xmlchar c)
  { return (isalnum(c) || c==':' || c=='-' || c=='_' || c=='.'); }

  //--------------------------------------------------------------------------
  // Read a character, skipping initial whitespace, and counting lines
  // Equivalent to s >> c, but with line counting
  // Existing character can be passed in for counting, too
  xmlchar skip_ws(istream &s, xmlchar c=0)
  { if (c=='\n') line++;
    for(;;) { c=0; s.get(c); 
              if (!isspace(c)) return c; if (c=='\n') line++; } }

  //--------------------------------------------------------------------------
  // Other private functions
  void parse_stream(istream &s) throw (ParseFailed);
  bool read_tag(xmlchar c, istream &s) throw(ParseFailed);
  void read_end_tag(xmlchar c, istream &s) throw(ParseFailed);
  void read_content(xmlchar c, istream &s) throw(ParseFailed);
  void read_ref(string& text, istream &s) throw (ParseFailed);
  string read_rest_of_name(xmlchar& c, istream& s);
  void skip_comment(istream &s) throw (ParseFailed);
  void skip_to_gt(istream &s) throw (ParseFailed);
  void skip_pi(istream &s) throw (ParseFailed);
  void error(const string& s);
  void fatal(const string& s) throw (ParseFailed);
  void initial_processing(Element *e);
  void final_processing(Element *e);
  void substitute_name(string& name, bool usedef=false);

protected:
  ostream& serr;       //error output stream
  int flags;

public:
  int errors;
  int line;

  //------------------------------------------------------------------------
  // Constructors & Destructor
  // s is output stream for parsing errors
  Parser(ostream &s, int f = PARSER_OPTIMISE_CONTENT):
    root(0),
    serr(s), 
    flags(f), 
    errors(0),
    line(1)
  {}

  // Default - use cerr
  Parser(int f = PARSER_OPTIMISE_CONTENT):
    root(0),
    serr(cerr), 
    flags(f),
    errors(0),
    line(1)
  {}

  ~Parser();

  //------------------------------------------------------------------------
  // Add namespace to prefix mapping
  // Any prefix used for this namespace in the document will be replaced
  // for the given prefix.  This allows you to normalise the tags you
  // check for
  void fix_namespace(const string& name, const string& prefix);

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

  //--------------------------------------------------------------------------
  // Real value fetch
  // Defaults to default value given (or 0.0) if not present
  // Returns 0.0 if present but bogus
  double get_value_real(const string& path, double def=0.0);

  //------------------------------------------------------------------------
  // Set value, either attribute or content of single (first) element
  // Returns whether value was settable
  // Can only set content or attributes of existing elements - use add_element
  // to create new ones. 
  bool set_value(const string& path, const string& value);

  //--------------------------------------------------------------------------
  // Boolean value set
  // Sets value to 'yes' or 'no'
  bool set_value_bool(const string& path, bool value);

  //--------------------------------------------------------------------------
  // Integer value set
  bool set_value_int(const string& path, int value);

  //--------------------------------------------------------------------------
  // Real value set
  bool set_value_real(const string& path, double value);

  //--------------------------------------------------------------------------
  // Delete the element(s) at the given path
  // Returns whether any such element existed
  bool delete_elements(const string& path);

  //--------------------------------------------------------------------------
  // Add an element below the given path
  // Takes the element and attaches to given path
  // Returns whether the parent element existed
  bool add_element(const string& path, Element *ne);

  //--------------------------------------------------------------------------
  // Add an element below the given path with given name
  // Creates empty element of given name below path
  // Returns new element if created, or 0 if parent didn't exist
  Element *add_element(const string& path, const string& name);

  //--------------------------------------------------------------------------
  // Ensure the given element path exists
  // Creates empty elements to fulfill the entire path if they don't already
  // exist.  Uses the first of any given name for path if more than one
  // Returns pointer to eventual child element (cannot fail)
  Element *ensure_path(const string& path);

  //--------------------------------------------------------------------------
  // Replace an element at the given path with the new one
  // Takes the element and attaches to given path, detachs and deletes the old
  // Returns whether the old element existed
  bool replace_element(const string& path, Element *ne);
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
  // No files - add later
  Configuration(int parse_flags=PARSER_OPTIMISE_CONTENT)
    :parser(parse_flags) {  }

  // Single filename
  Configuration(const string& fn, int parse_flags=PARSER_OPTIMISE_CONTENT)
    :parser(parse_flags) { filenames.push_back(fn); }

  // List of filenames - front() is tried first
  Configuration(list<string>& fns, int parse_flags=PARSER_OPTIMISE_CONTENT)
    :filenames(fns), parser(parse_flags) {}

  //------------------------------------------------------------------------
  // Add a filename to the config list, post creation
  void add_file(const string& fn) { filenames.push_back(fn); }

  //------------------------------------------------------------------------
  // Add namespace to prefix mapping (see Parser::fix_namespace)
  void fix_namespace(const char *name, const char *prefix)
  { parser.fix_namespace(name, prefix); }

  //------------------------------------------------------------------------
  // Read configuration file
  // Returns whether successful
  // ename is the expected root element name - fails if wrong
  bool read(const string& ename, ostream& err=cerr);

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

  //--------------------------------------------------------------------------
  // XPath Real value fetch
  // Defaults to default value given (or 0.0) if not present
  // Returns 0.0 if present but bogus
  double get_value_real(const string& path, double def=0.0);

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

  //------------------------------------------------------------------------
  // XPath value set - either attribute or content of single (first) element
  // Returns whether value was settable
  // Can only set content or attributes of existing elements - use add_element
  // to create new ones. 
  bool set_value(const string& path, const string& value);

  //--------------------------------------------------------------------------
  // XPath Boolean value set - sets to 'yes' or 'no'
  bool set_value_bool(const string& path, bool value);

  //--------------------------------------------------------------------------
  // Integer value set
  bool set_value_int(const string& path, int value);

  //--------------------------------------------------------------------------
  // Real value set
  bool set_value_real(const string& path, double value);

  //--------------------------------------------------------------------------
  // Delete the element(s) at the given path
  // Returns whether any such element existed
  bool delete_elements(const string& path);

  //--------------------------------------------------------------------------
  // Add an element below the given path
  // Takes the element and attaches to given path
  // Returns whether the parent element existed
  bool add_element(const string& path, Element *ne);

  //--------------------------------------------------------------------------
  // Add an element below the given path with given name
  // Creates empty element of given name below path
  // Returns new element if created, or 0 if parent didn't exist
  Element *add_element(const string& path, const string& name);

  //--------------------------------------------------------------------------
  // Ensure the given element path exists
  // Creates empty elements to fulfill the entire path if they don't already
  // exist.  Uses the first of any given name for path if more than one
  // Returns pointer to eventual child element (cannot fail)
  Element *ensure_path(const string& path);

  //--------------------------------------------------------------------------
  // Replace an element at the given path with the new one
  // Takes the element and attaches to given path, detachs and deletes the old
  // Returns whether the old element existed
  bool replace_element(const string& path, Element *ne);

  //------------------------------------------------------------------------
  // Write back file from changes made to in-memory document
  // Writes back to first (or only) file in filename list
  // Returns whether successful
  // NB: All comments are lost!
  bool write();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XML_H



