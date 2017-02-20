//==========================================================================
/// \file obtools/libs/xml/ot-xml.h
/// Public definitions for ObTools XML parser
///
/// Copyright (c) 2003-2007 Paul Clark.  All rights reserved
/// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_XML_H
#define __OBTOOLS_XML_H

#include <string>
#include <deque>
#include <list>
#include <map>
#include <iostream>
#include <stdint.h>

namespace ObTools {

/// %XML parser/DOM/XPath library.
/// Provides fairly complete but non-standard DOM
/// and minimal XPath for configuration files
namespace XML {

// Make our lives easier without polluting anyone else
using namespace std;

/// Type to use for characters
typedef char xmlchar;

class Element; // Forward

//==========================================================================
/// %Element list iterator.
/// Actually contains a list of elements;  sequential forward access only.
///
/// \note Usage is different from STL iterator - e.g.
/// \code
///    for(Element::iterator p(parent.children); p; ++p)
///       XML::Element& child = *p;
/// \endcode
struct ElementIterator
{
  list<Element *> elements;              ///< List we're iterating
  list<Element *>::const_iterator it;    ///< Our internal iterator

  //------------------------------------------------------------------------
  /// Constructor from element list
  ElementIterator(const list<Element *>& l):
    elements(l), it(elements.begin()) {}

  /// Copy constructor
  ElementIterator(const ElementIterator& o):
    elements(o.elements), it(elements.begin()) {}

  //------------------------------------------------------------------------
  /// Validity check
  bool valid() { return it != elements.end(); }
  operator bool() { return valid(); }     ///< Cast to bool - checks validity
  bool operator!() { return !valid(); }   ///< Not operator - checks invalidity

  //------------------------------------------------------------------------
  /// Incrementor.
  /// Postincrement (p++) is not provided because it would be very inefficient
  ElementIterator& operator++() { it++; return *this; }

  //------------------------------------------------------------------------
  // Dereference operators
  Element& operator*() const { return **it; }  ///< Retrieve element
  Element *operator->() const { return *it; }  ///< Access element through ptr
};

/// Const equivalent to ElementIterator
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
/// %XML %Parser flags
enum
{
  /// Snap single text content elements back to parent-element.content.
  /// Makes simple grammars a lot easier to access
  PARSER_OPTIMISE_CONTENT=1,

  /// Preserve whitespace as it is. Prepare for a flood of indentation strings!
  /// \note If you use this, the << >> process isn't round-trip clean for
  /// whitespace, because the output adds its own indentation
  PARSER_PRESERVE_WHITESPACE=2,

  /// Do namespace translation.
  /// Automatically turned on if you call fix_namespace
  PARSER_FIX_NAMESPACES=4,

  /// Be lenient with & and < in contexts in which they couldn't be
  /// %XML syntax.
  /// i.e., not followed by a name character, '#' (for &),
  /// '!', '?' or '/' (for <) - and consider them just normal character data.
  /// This makes it more pleasant to include code, particularly C++ code,
  /// in %XML
  ///
  /// \note This mimics the behaviour of SGML (hooray!) (ISO 8879:B.7.3), but
  /// strictly violates %XML (XML1.0:2.4), unless you consider it a form
  /// of error handling...  But seriously, folks, using this feature may
  /// mean your %XML files are rejected by parsers who are less lenient
  /// (or by this parser without this flag set)
  PARSER_BE_LENIENT=8
};

//==========================================================================
/// %XML %Element class.
/// An %XML document is a tree of these
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

public:
  /// %Element name ('tag') - empty for data 'elements'
  string name;

  /// %Element content. Set only for data 'elements', or if 'OPTIMISE'
  /// flag set, when single data elements are snapped back to the content
  /// of the parent
  string content;

  /// Attributes - map of attr-name -> attr-value.
  /// \note Beware of using [] to access this - will modify map and add empty
  /// attributes if it doesn't exist.  Use get_attr() in preference if
  /// you need to restream the input
  map<string,string> attrs;

  /// List of sub-elements
  list<Element *> children;

  /// Parent element
  Element *parent;

  // Error support
  int line;   ///< Line number of our start tag

  //------------------------------------------------------------------------
  /// Non-element marker
  static Element none;

  //------------------------------------------------------------------------
  // Iterator defs
  typedef ElementIterator iterator;            ///< Typedef of element iterator
  typedef ConstElementIterator const_iterator; ///< Typedef of const iterator

  //------------------------------------------------------------------------
  /// Validity check.
  /// Easy way of checking if you've got Element::none
  /// \note Don't use \code(e!=Element::none)\endcode - that compares values!
  /// \note We don't provide an operator bool due to problems with ambiguity
  /// of [] operator
  operator bool() const { return valid(); }
  bool valid() const { return this!=&none; }
  bool operator!() const { return !valid(); }  ///< Invalidity check

  //------------------------------------------------------------------------
  // Constructors
  /// Default constructor - no name, no content, for later filling in
  Element():parent(0), line(0) { }

  /// Constructor with one string: just the name
  Element(const string& n):name(n), parent(0), line(0) { }

  /// Constructor with two strings: name and textual content
  Element(const string& n, const string& c):
    name(n), content(c), parent(0), line(0) {  }

  /// Constructor with three strings: name and one attribute - e.g. namespace
  Element(const string& n, const string& a, const string& v):
    name(n), parent(0), line(0){ set_attr(a,v); }

  /// Constructor with four strings: name, one attribute and content
  Element(const string& n, const string& a, const string& v, const string& c):
    name(n), content(c), parent(0), line(0) { set_attr(a,v); }

  //------------------------------------------------------------------------
  /// Shallow copy to an existing element.
  /// Copies the name, direct content and attributes into the given element
  /// Children, parent and line are _not_ copied
  void copy_to(Element& dest) const
  { dest.name = name; dest.content = content;  dest.attrs = attrs; }

  //------------------------------------------------------------------------
  /// Shallow copy to a new element
  Element *copy() const
  { Element *dest = new Element(); copy_to(*dest); return dest; }

  //------------------------------------------------------------------------
  /// Deep copy to an existing element.
  /// Copies the name, direct content and attributes into the given element
  /// and recursively copies children
  // parent pointer of top element is not copied
  void deep_copy_to(XML::Element& dest) const;

  //------------------------------------------------------------------------
  /// Deep copy to a new element
  Element *deep_copy() const
  { Element *dest = new Element(); deep_copy_to(*dest); return dest; }

  //------------------------------------------------------------------------
  /// Copy constructor.  Does a deep copy
  Element(const Element& src): parent(0), line(0)
  { src.deep_copy_to(*this); }

  //------------------------------------------------------------------------
  /// Assignment operator.  Does a deep copy, deleting any existing children
  const Element& operator=(const Element& src)
  { parent=0; line=src.line;
    clear_children(); src.deep_copy_to(*this); return src; }

  //------------------------------------------------------------------------
  // Superimpose an element on top of this one
  // Any attributes or children from the given child are added to this
  // element, replacing any existing data where the attribute/child matches
  // the original. This happens recursively through the element's children.
  // An identifier attribute can be specified to determine uniqueness by. If
  // left empty, the element name is used.
  void superimpose(const Element& source, const string& identifier="");

  //------------------------------------------------------------------------
  // Merge with another element
  // The attributes and content of the source element are copied into this
  // element, adding to or replacing (attributes only) what was there before
  // This element's name, content and parent pointer are not changed
  void merge(const Element& source);

  //------------------------------------------------------------------------
  /// Add a child element, taking ownership
  Element& add(Element *child)
  { children.push_back(child); child->parent = this; return *child; }

  //------------------------------------------------------------------------
  /// Add a child element from a reference, copying
  Element& add(const Element &child)
  { return add(child.deep_copy()); }

  //------------------------------------------------------------------------
  /// Add a new empty child element by name
  /// \return New element
  Element& add(const string& n) { return add(new Element(n)); }

  /// Add with two strings: name and textual content
  /// \return New element
  Element& add(const string& n, const string& c)
  { return add(new Element(n, c)); }

  /// Add with three strings: name and one attribute - e.g. namespace
  /// \return New element
  Element& add(const string& n, const string& a, const string& v)
  { return add(new Element(n, a, v)); }

  /// Add with four strings: name, one attribute and content
  /// \return New element
  Element& add(const string& n, const string& a,
               const string& v, const string& c)
  { return add(new Element(n, a, v, c)); }

  //------------------------------------------------------------------------
  /// Add child elements from XML text.  Reparses text and adds resulting
  /// root as a child element.  ostream & parse_flags as Parser() below
  /// \return Added child, or 'none' if parse failed
  Element& add_xml(const string& xml, ostream& serr = cerr,
                   int parse_flags = PARSER_OPTIMISE_CONTENT);

  //------------------------------------------------------------------------
  // Merge from XML text - reparses text and merges resulting element
  // with this one (see merge() for details).
  // Parsed element must be the same name as this one
  // Parser ostream & flags as Parser() below
  // Returns whether parse succeeded and element was the same name
  bool merge_xml(const string& xml, ostream& serr = cerr,
                 int parse_flags = PARSER_OPTIMISE_CONTENT);

  //------------------------------------------------------------------------
  /// Dump to given output stream.
  /// \param s stream to output to
  /// \param with_pi controls whether to including the standard-compliant
  /// <?xml .. ?>
  void write_to(ostream& s, bool with_pi=false) const;

  //------------------------------------------------------------------------
  /// Convert to a string.
  /// \param with_pi controls whether to including the standard-compliant
  /// <?xml .. ?>
  string to_string(bool with_pi=false) const;

  //------------------------------------------------------------------------
  /// Write start-tag only to a given stream.
  /// \note Always outputs unclosed start tag, even if empty
  void write_start_to(ostream &s) const;

  //------------------------------------------------------------------------
  /// Convert start-tag to a string
  string start_to_string() const;

  //------------------------------------------------------------------------
  /// Write end-tag only to a given stream
  void write_end_to(ostream &s) const;

  //------------------------------------------------------------------------
  /// Convert end-tag to a string
  string end_to_string() const;

  //------------------------------------------------------------------------
  /// 'Optimise' single text sub-elements back to 'content' string here
  void optimise();

  //------------------------------------------------------------------------
  /// Find n'th (first, by default) child element, whatever it is
  /// \return Element::none if there isn't one
  Element& get_child(int n=0);
  const Element& get_child(int n=0) const;      ///< \copydoc get_child(int)

  //------------------------------------------------------------------------
  /// Find n'th (first, by default) child element, but ignoring text/WS
  /// \return Element::none if there isn't one
  Element& get_child_element(int n=0);
  const Element& get_child_element(int n=0) const;
                                         ///< \copydoc get_child_element(int)

  //------------------------------------------------------------------------
  /// Find n'th (first, by default) child element of given name.
  /// \return Element::none if there isn't one
  /// \note Use valid() to check which you've got - e.g.:
  /// \code
  ///     XML::Element& e = get_child(root, "foo");
  ///     if (e.valid())
  ///       //Use e
  /// \endcode
  const Element& get_child(const string& ename, int n=0) const;
  Element& get_child(const string& ename, int n=0);

  //------------------------------------------------------------------------
  // Ensure the existence of a child of the given name, and return it
  // Creates new child element of the given name if one doesn't already exist
  Element& make_child(const string& ename);

  //------------------------------------------------------------------------
  // Find first (or only) descendant of given name
  // Returns Element::none if there isn't one
  // (Like get_child() but ignoring intervening cruft)
  const Element& get_descendant(const string& ename) const;
  Element& get_descendant(const string& ename);

  //------------------------------------------------------------------------
  // Find all child elements in a list of const elements (so it can be used
  // with a const_iterator)
  // For non-const use 'children' directly
  // Returns copy list of pointers
  list<const Element *> get_children() const;

  //------------------------------------------------------------------------
  // Find all child elements of given name
  // Returns list of pointers
  list<const Element *> get_children(const string& ename) const;
  list<Element *> get_children(const string& ename);

  //------------------------------------------------------------------------
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

  //------------------------------------------------------------------------
  // Get an attribute of the given name
  // Returns attribute value
  // Defaults to default value given (or "") if not present
  // This exists to avoid creating the attribute when using e.attrs["foo"]
  // when foo doesn't exist - use e["foo"] instead
  string get_attr(const string& attname, const string& def="") const;

  //------------------------------------------------------------------------
  // Handy [] operator to get attribute values
  string operator[](const char *attr) const { return get_attr(attr); }
  string operator[](const string& attr) const { return get_attr(attr); }

  //------------------------------------------------------------------------
  // Get the boolean value of an attribute of the given name
  // Returns attribute value
  // Defaults to default value given (or false) if not present
  // Recognises words beginning [TtYy1] as true, everything else is false
  bool get_attr_bool(const string& attname, bool def=false) const;

  //------------------------------------------------------------------------
  // Get the integer value of an attribute of the given name
  // Returns attribute value
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  int get_attr_int(const string& attname, int def=0) const;

  //------------------------------------------------------------------------
  // Get the integer value of an attribute of the given name, from hex string
  // Returns attribute value
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  int get_attr_hex(const string& attname, int def=0) const;

  //------------------------------------------------------------------------
  // Get the 64-bit integer value of an attribute of the given name
  // Returns attribute value
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  uint64_t get_attr_int64(const string& attname, uint64_t def=0) const;

  //------------------------------------------------------------------------
  // Get the 64-bit integer value of an attribute of the given name, from hex
  // string
  // Returns attribute value
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  uint64_t get_attr_hex64(const string& attname, uint64_t def=0) const;

  //------------------------------------------------------------------------
  // Get the real value of an attribute of the given name
  // Returns attribute value
  // Defaults to default value given (or 0.0) if not present
  // Returns 0.0 if present but bogus
  double get_attr_real(const string& attname, double def=0.0) const;

  //------------------------------------------------------------------------
  // Tests whether the element has an attribute of the given name
  // Quicker than !get_attr("foo").empty()
  bool has_attr(const string& attname) const;

  //------------------------------------------------------------------------
  // Set an attribute (string)
  // Note:  All set_attr_xxx methods return *this, to allow chaining
  Element& set_attr(const string& attname, const string& value);

  //------------------------------------------------------------------------
  // Set an attribute (integer)
  // (_int qualifier not strictly necessary here, but matches get_attr_int)
  Element& set_attr_int(const string& attname, int value);

  //------------------------------------------------------------------------
  // Set an attribute (integer, hex)
  // (_int qualifier not strictly necessary here, but matches get_attr_int)
  Element& set_attr_hex(const string& attname, int value);

  //------------------------------------------------------------------------
  // Set an attribute (64-bit integer)
  // (_int64 qualifier not strictly necessary here, but matches get_attr_int64)
  Element& set_attr_int64(const string& attname, uint64_t value);

  //------------------------------------------------------------------------
  // Set an attribute (64-bit integer, hex)
  Element& set_attr_hex64(const string& attname, uint64_t value);

  //------------------------------------------------------------------------
  // Set an attribute (bool)
  // (_bool qualifier not strictly necessary here, but matches get_attr_bool)
  Element& set_attr_bool(const string& attname, bool value);

  //------------------------------------------------------------------------
  // Set an attribute (real)
  // (_real qualifier not strictly necessary here, but matches get_attr_real)
  Element& set_attr_real(const string& attname, double value);

  //------------------------------------------------------------------------
  // Remove an attribute
  Element& remove_attr(const string& attname);

  //------------------------------------------------------------------------
  // Get all direct child text content accumulated into one string
  // Returns optimised content if available, otherwise iterates children
  // collecting text from data elements
  // Strings from separate elements are separately with '\n'
  string get_content() const;

  //------------------------------------------------------------------------
  // Handy * operator to get content
  string operator*() const { return get_content(); }

  //------------------------------------------------------------------------
  // Get all text content from the entire tree accumulated into one string
  // Returns optimised content if available, otherwise iterates children
  // collecting text from data elements, and recursing into subchildren
  // Strings from separate elements are separately with '\n'
  string get_deep_content() const;

  //------------------------------------------------------------------------
  // Get XPath position relative to root (not including the root itself)
  // Returns an XPath string that can be used to identify this element in
  // the same document
  string get_xpath() const;

  //------------------------------------------------------------------------
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
  // Detach from parent
  void detach();

  //------------------------------------------------------------------------
  // Replace with the given element at same position in parent
  // Detaches this element and attaches the new one
  void replace_with(Element *e);

  //------------------------------------------------------------------------
  // Remove children of the given name
  // Recursively destroys children
  void remove_children(const string& name);

  //------------------------------------------------------------------------
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
  // Persistent state
  map<string, string> user_ns_map;  //Map of full name->prefix

  // Transient per-document state
  deque<Element *> elements;  //Could be a <stack>, but no clear()
  Element *root;       //0 if not valid
  deque<map<string, string> > ns_maps; //Stack of maps of prefix->full name

  //------------------------------------------------------------------------
  // Inline character classification functions
  // Note: Only allows strict ascii
  bool is_name_start(xmlchar c)
  { return (isascii(c) && isalnum(c))||c==':'||c=='_'; }
  bool is_name_char(xmlchar c)
  { return (isascii(c) && isalnum(c))||c==':'||c=='-'||c=='_'||c=='.'; }

  // Safe isspace including ASCII test - prevents treating (e.g.) 0xa0 as
  // space when in Windows ANSI locale
  bool is_ascii_space(xmlchar c) { return isascii(c) && isspace(c); }

  //------------------------------------------------------------------------
  // Read a character, skipping initial whitespace, and counting lines
  // Equivalent to s >> c, but with line counting
  // Existing character can be passed in for counting, too
  xmlchar skip_ws(istream &s, xmlchar c=0)
  { if (c=='\n') line++;
    for(;;) { c=0; s.get(c);
              if (!is_ascii_space(c)) return c; if (c=='\n') line++; } }

  //------------------------------------------------------------------------
  // Other private functions
  void parse_stream(istream &s) throw (ParseFailed);
  bool read_tag(xmlchar c, istream &s) throw(ParseFailed);
  void read_end_tag(xmlchar c, istream &s) throw(ParseFailed);
  void read_content(xmlchar c, istream &s) throw(ParseFailed);
  void read_ref(string& text, istream &s) throw (ParseFailed);
  void read_rest_of_name(xmlchar& c, istream& s, string& name);
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
  Element& get_root() const;

  //------------------------------------------------------------------------
  // Get root element detached from parser, so you can keep it after the
  // parser has died
  // Returns 0 if not valid
  Element *detach_root();

  //------------------------------------------------------------------------
  // Detach and delete the old root (if any) and replace with a new one
  // Element is taken and will be deleted by parser if not detached/replaced
  // itself
  void replace_root(Element *e);
};


//==========================================================================
// XML stream operators

//--------------------------------------------------------------------------
// >> operator to parse from istream
//
// e.g. cin >> parser;
//
// Throws ParseFailed if bad XML received
istream& operator>>(istream& s, Parser& p) throw (ParseFailed);

//--------------------------------------------------------------------------
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

//--------------------------------------------------------------------------
// Base template which can be specialised either as const or non-const
// Because we only need the two specialisations these are done in xpath.cc
// which allows us to put the implementation in there too
template<class ELEMENT> class BaseXPathProcessor
{
protected:
  ELEMENT& root;           // Document root we're working on

public:
  //------------------------------------------------------------------------
  // Constructors
  BaseXPathProcessor(): root(Element::none) {}
  BaseXPathProcessor(ELEMENT& _root): root(_root) {}

  //------------------------------------------------------------------------
  // Element list fetch - all elements matching final child step.
  // Only first element of intermediate steps is used - list is not merged!
  list<ELEMENT *> get_elements(const string& path) const;

  //------------------------------------------------------------------------
  // Single element fetch - first of list, if any, or 0
  ELEMENT *get_element(const string& path) const;

  //------------------------------------------------------------------------
  // Value fetch - either attribute or content of single (first) element
  // Returns def if anything not found
  string get_value(const string& path, const string& def="") const;

  //------------------------------------------------------------------------
  // [] operator to make things easy
  // e.g. xpath["foo/bar"]
  string operator[](const string& path) const { return get_value(path); }

  //------------------------------------------------------------------------
  // Boolean value fetch
  // Defaults to default value given (or false) if not present
  // Recognises words beginning [TtYy] as true, everything else is false
  bool get_value_bool(const string& path, bool def=false) const;

  //------------------------------------------------------------------------
  // Integer value fetch
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  int get_value_int(const string& path, int def=0) const;

  //------------------------------------------------------------------------
  // Hex value fetch
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  int get_value_hex(const string& path, int def=0) const;

  //------------------------------------------------------------------------
  // 64-bit integer value fetch
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  uint64_t get_value_int64(const string& path, uint64_t def=0) const;

  //------------------------------------------------------------------------
  // 64-bit integer value fetch from hex
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  uint64_t get_value_hex64(const string& path, uint64_t def=0) const;

  //------------------------------------------------------------------------
  // Real value fetch
  // Defaults to default value given (or 0.0) if not present
  // Returns 0.0 if present but bogus
  double get_value_real(const string& path, double def=0.0) const;
};

//--------------------------------------------------------------------------
// Const XPathProcessor which only provides read functionality
class ConstXPathProcessor: public BaseXPathProcessor<const Element>
{
public:
  //------------------------------------------------------------------------
  // Constructors
  ConstXPathProcessor(): BaseXPathProcessor<const Element>() {}
  ConstXPathProcessor(const Element& _root):
    BaseXPathProcessor<const Element>(_root) {}
};

//--------------------------------------------------------------------------
// Read-write XPathProcessor which offers set methods etc. as well
class XPathProcessor: public BaseXPathProcessor<Element>
{
public:
  //------------------------------------------------------------------------
  // Constructors
  XPathProcessor(): BaseXPathProcessor<Element>() {}
  XPathProcessor(Element& _root):
    BaseXPathProcessor<Element>(_root) {}

  //------------------------------------------------------------------------
  // Set value, either attribute or content of single (first) element
  // Returns whether value was settable
  // Can only set content or attributes of existing elements - use add_element
  // to create new ones.
  bool set_value(const string& path, const string& value);

  //------------------------------------------------------------------------
  // Boolean value set
  // Sets value to 'yes' or 'no'
  bool set_value_bool(const string& path, bool value);

  //------------------------------------------------------------------------
  // Integer value set
  bool set_value_int(const string& path, int value);

  //------------------------------------------------------------------------
  // Integer value set to hex
  bool set_value_hex(const string& path, int value);

  //------------------------------------------------------------------------
  // 64-bit integer value set
  bool set_value_int64(const string& path, uint64_t value);

  //------------------------------------------------------------------------
  // 64-bit integer value set to hex
  bool set_value_hex64(const string& path, uint64_t value);

  //------------------------------------------------------------------------
  // Real value set
  bool set_value_real(const string& path, double value);

  //------------------------------------------------------------------------
  // Delete the element(s) at the given path
  // Returns whether any such element existed
  bool delete_elements(const string& path);

  //------------------------------------------------------------------------
  // Add an element below the given path
  // Takes the element and attaches to given path
  // Returns whether the parent element existed
  bool add_element(const string& path, Element *ne);

  //------------------------------------------------------------------------
  // Add an element below the given path with given name
  // Creates empty element of given name below path
  // Returns new element if created, or 0 if parent didn't exist
  Element *add_element(const string& path, const string& name);

  //------------------------------------------------------------------------
  // Ensure the given element path exists
  // Creates empty elements to fulfill the entire path if they don't already
  // exist.  Uses the first of any given name for path if more than one
  // Returns pointer to eventual child element (cannot fail)
  Element *ensure_path(const string& path);

  //------------------------------------------------------------------------
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

protected:
  ostream& serr;  // Error output stream

public:
  //------------------------------------------------------------------------
  // Constructors
  // No files - add later
  Configuration(ostream& _serr=cerr, int parse_flags=PARSER_OPTIMISE_CONTENT)
    :parser(_serr, parse_flags), serr(_serr) {  }

  // Single filename
  Configuration(const string& fn, ostream& _serr=cerr,
                int parse_flags=PARSER_OPTIMISE_CONTENT)
    :parser(_serr, parse_flags), serr(_serr) { filenames.push_back(fn); }

  // List of filenames - front() is tried first
  Configuration(list<string>& fns, ostream& _serr=cerr,
                int parse_flags=PARSER_OPTIMISE_CONTENT)
    :filenames(fns), parser(_serr, parse_flags), serr(_serr) {}

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
  // If specified, ename is the expected root element name - fails if wrong
  bool read(const string& ename="");

  //------------------------------------------------------------------------
  // Read from XML text
  // If specified, ename is the expected root element name - fails if wrong
  bool read_text(const string& text, const string& ename="");

  //------------------------------------------------------------------------
  // Reload configuration file as the same element as before
  // Returns whether successful
  bool reload();

  //--------------------------------------------------------------------------
  // Superimpose XML from the given file
  void superimpose_file(const string& fn, bool allow_includes=false);

  //--------------------------------------------------------------------------
  // Process include files
  // Reads
  //   <include file="..."/>
  // from top level of document.  File can be relative to this file's path
  // and can contain a leaf wildcard.
  // XML from included files is superimposed in order
  void process_includes();

  //------------------------------------------------------------------------
  // Get root element
  // Returns Element::none if not valid
  Element& get_root() const { return parser.get_root(); }

  //------------------------------------------------------------------------
  // Detach the root element to keep after the configuration reader has died
  // Returns 0 if not valid
  Element *detach_root() { return parser.detach_root(); }

  //------------------------------------------------------------------------
  // Element list fetch - all elements matching final child step.
  // Only first element of intermediate steps is used - list is not merged!
  list<Element *> get_elements(const string& path) const;

  //------------------------------------------------------------------------
  // Single element fetch - first of list, if any, or 0
  Element *get_element(const string& path) const;

  //------------------------------------------------------------------------
  // XPath value fetch - either attribute or content of single (first) element
  // Returns def if anything not found
  // Note all get_value methods still work, and return def, if file read fails
  string get_value(const string& path, const string& def="") const;

  //------------------------------------------------------------------------
  // [] operator to make things easy
  // e.g. config["foo/bar"]
  string operator[](const string& path) { return get_value(path); }

  //------------------------------------------------------------------------
  // XPath Boolean value fetch
  // Defaults to default value given (or false) if not present
  // Recognises words beginning [TtYy] as true, everything else is false
  bool get_value_bool(const string& path, bool def=false) const;

  //------------------------------------------------------------------------
  // XPath Integer value fetch
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  int get_value_int(const string& path, int def=0) const;

  //------------------------------------------------------------------------
  // Integer hex value fetch
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  int get_value_hex(const string& path, int def=0) const;

  //------------------------------------------------------------------------
  // 64-bit integer value fetch
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  uint64_t get_value_int64(const string& path, uint64_t def=0) const;

  //------------------------------------------------------------------------
  // 64-bit integer hex value fetch
  // Defaults to default value given (or 0) if not present
  // Returns 0 if present but bogus
  uint64_t get_value_hex64(const string& path, uint64_t def=0) const;

  //------------------------------------------------------------------------
  // XPath Real value fetch
  // Defaults to default value given (or 0.0) if not present
  // Returns 0.0 if present but bogus
  double get_value_real(const string& path, double def=0.0) const;

  //------------------------------------------------------------------------
  // XPath list-of-values fetch
  // Returns contents of all elements matching XPath
  list<string> get_values(const string& path) const;

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

  //------------------------------------------------------------------------
  // XPath Boolean value set - sets to 'yes' or 'no'
  bool set_value_bool(const string& path, bool value);

  //------------------------------------------------------------------------
  // Integer value set
  bool set_value_int(const string& path, int value);

  //------------------------------------------------------------------------
  // Integer value set to hex
  bool set_value_hex(const string& path, int value);

  //------------------------------------------------------------------------
  // 64-bit integer value set
  bool set_value_int64(const string& path, uint64_t value);

  //------------------------------------------------------------------------
  // 64-bit integer value set to hex
  bool set_value_hex64(const string& path, uint64_t value);

  //------------------------------------------------------------------------
  // Real value set
  bool set_value_real(const string& path, double value);

  //------------------------------------------------------------------------
  // Delete the element(s) at the given path
  // Returns whether any such element existed
  bool delete_elements(const string& path);

  //------------------------------------------------------------------------
  // Add an element below the given path
  // Takes the element and attaches to given path
  // Returns whether the parent element existed
  bool add_element(const string& path, Element *ne);

  //------------------------------------------------------------------------
  // Add an element below the given path with given name
  // Creates empty element of given name below path
  // Returns new element if created, or 0 if parent didn't exist
  Element *add_element(const string& path, const string& name);

  //------------------------------------------------------------------------
  // Ensure the given element path exists
  // Creates empty elements to fulfill the entire path if they don't already
  // exist.  Uses the first of any given name for path if more than one
  // Returns pointer to eventual child element (cannot fail)
  Element *ensure_path(const string& path);

  //------------------------------------------------------------------------
  // Replace an element at the given path with the new one
  // Takes the element and attaches to given path, detachs and deletes the old
  // Returns whether the old element existed
  bool replace_element(const string& path, Element *ne);

  //------------------------------------------------------------------------
  // Replace the root (if any) with a new one of the given name
  // Returns the new root
  Element *replace_root(const string& name);

  //------------------------------------------------------------------------
  // Move to a different location - use after renaming the file underneath
  // it, to ensure write() works on the new location
  void move_file(const string& fn)
  { filenames.clear(); filenames.push_back(fn); }

  //------------------------------------------------------------------------
  // Write back file from changes made to in-memory document
  // Writes back to first (or only) file in filename list
  // Returns whether successful
  // NB: All comments are lost!
  bool write();
};

//==========================================================================
// Text expander
// Expands text using a simple tag language from an XML document giving
// expansion values
// 'value's are XPath expressions in the value document

//   <expand:replace value|var="xxx"/>
//     Expands to value of XPath or variable 'xxx' or "" if not present

//   <expand:if value|var="xxx">
//     Expands children if XPath or variable 'xxx' is present and begins
//     with [YyTt1]

//   <expand:unless value|var="xxx">
//     Expands children unless XPath or variable 'xxx' is present and begins
//     with [YyTt1]

//   <expand:ifeq value|var="xxx" to="yyy">
//     Expands children if XPath or variable 'xxx' is equal (cased) to 'yyy'

//   <expand:ifne value|var="xxx" to="yyy">
//     Expands children if XPath or variable 'xxx' is !equal (cased) to 'yyy'

//   <expand:each element="xxx">
//     Expands children for every XPath 'xxx', making the element the new
//     context

//   <expand:index [from="1"]/>
//     Expands to loop index value, optionally from a given base (default 1)

//   <expand:set var="xxx">
//     Sets variable 'xxx' to the content of the element (expanded)
//     Variables have scope within the local 'each' context

// All other child elements are output verbatim

class Expander
{
  const XML::Element& templ;

  // Internal
  string expand_recursive(const XML::Element& templ,
                          XML::Element& values,
                          int index,
                          map<string, string>& vars);

public:
  //------------------------------------------------------------------------
  // Constructor - takes template XML document
  Expander(const XML::Element& _templ): templ(_templ) {}

  //------------------------------------------------------------------------
  // Expand the template with the given value document
  // \return the expanded string
  string expand(XML::Element& values);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XML_H



