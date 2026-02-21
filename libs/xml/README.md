# ObTools::XML

A custom XML parser, DOM, XPath processor, and template expander for C++17.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Features

- **Streaming parser** with optional content optimisation, lenient mode, and namespace normalisation
- **Non-standard DOM** optimised for configuration files and simple grammars
- **Simplified XPath** for reading and writing element/attribute values with type conversion
- **Configuration file manager** with file fallback, include processing, and atomic write-back
- **Template expander** with loops, conditionals, and variable scoping

## Dependencies

- `ot-file` - File I/O (used by Configuration)
- `ot-log` - Logging

## Quick Start

Include the header:

```cpp
#include "ot-xml.h"
using namespace ObTools;
```

### Parsing XML

```cpp
// Parse from a string
XML::Parser parser;
try
{
  parser.read_from("<bookstore>"
                   "  <book lang='en'>"
                   "    <title>The C++ Programming Language</title>"
                   "    <price>59.99</price>"
                   "  </book>"
                   "  <book lang='en'>"
                   "    <title>Effective Modern C++</title>"
                   "    <price>45.00</price>"
                   "  </book>"
                   "</bookstore>");
}
catch (const XML::ParseFailed&)
{
  cerr << "Parse failed with " << parser.errors << " errors\n";
  return;
}

XML::Element& root = parser.get_root();
```

```cpp
// Parse from a stream
XML::Parser parser;
ifstream file("data.xml");
file >> parser;
XML::Element& root = parser.get_root();
```

### Navigating the DOM

```cpp
// Get a named child element
XML::Element& book = root.get_child("book");
if (book.valid())
{
  // Read attributes
  string lang = book["lang"];           // "en"
  string lang2 = book.get_attr("lang"); // equivalent

  // Read text content (with content optimisation)
  XML::Element& title = book.get_child("title");
  string name = *title;                 // "The C++ Programming Language"
  string name2 = title.get_content();   // equivalent
}

// Get the n'th child of a given name (0-indexed)
XML::Element& second_book = root.get_child("book", 1);

// Skip text/whitespace nodes
XML::Element& first_element = root.get_child_element(0);

// Find a descendant anywhere in the subtree
XML::Element& price = root.get_descendant("price");

// Get all children with a given name
list<Element *> books = root.get_children("book");
for (auto *b : books)
  cout << b->get_child("title").get_content() << endl;
```

### Iterating Children

```cpp
// Using ElementIterator (preferred)
for (XML::Element::iterator it(root.children); it; ++it)
{
  XML::Element& child = *it;
  cout << child.name << endl;
}

// Iterate only named children
for (XML::Element::iterator it(root.get_children("book")); it; ++it)
  cout << *it->get_child("title") << endl;

// Using the legacy macros (deprecated, but still works)
OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(book, root, "book")
  cout << book.get_child("title").get_content() << endl;
OBTOOLS_XML_ENDFOR
```

### Building Elements Programmatically

```cpp
// Create a document from scratch
XML::Element root("config");
root.set_attr("version", "1.0");

// Add children with chaining
root.add("server", "host", "localhost")
    .set_attr_int("port", 8080)
    .set_attr_bool("ssl", true);

root.add("database")
    .set_attr("driver", "postgresql")
    .add("connection", "host", "db.example.com");

// Add content
root.add("log-file", "/var/log/app.log");

// Output
cout << root;
// Produces:
// <config version="1.0">
//   <server host="localhost" port="8080" ssl="yes"/>
//   <database driver="postgresql">
//     <connection host="db.example.com"/>
//   </database>
//   <log-file>/var/log/app.log</log-file>
// </config>
```

### Typed Attribute Access

```cpp
XML::Element& server = root.get_child("server");

// Getters with type conversion and defaults
string host = server.get_attr("host", "127.0.0.1");
int port    = server.get_attr_int("port", 80);
bool ssl    = server.get_attr_bool("ssl", false);
double rate = server.get_attr_real("rate", 1.0);
int flags   = server.get_attr_hex("flags", 0);
uint64_t id = server.get_attr_int64("id", 0);

// Check existence
if (server.has_attr("timeout"))
  int timeout = server.get_attr_int("timeout");

// Get all attributes with a common prefix
// e.g. <item opt-x="1" opt-y="2" name="foo"/>
map<string, string> opts = item.get_attrs_with_prefix("opt-");
// opts = { "x": "1", "y": "2" }

// Setters (all return *this for chaining)
server.set_attr("host", "0.0.0.0")
      .set_attr_int("port", 443)
      .set_attr_bool("ssl", true)
      .set_attr_real("rate", 2.5);
```

### XPath Queries

The XPath processor supports a simplified subset: child element steps, indexed access, and attribute selection.

```cpp
XML::XPathProcessor xpath(root);

// Element content
string title = xpath["book/title"];            // text content of first match
string title2 = xpath.get_value("book/title"); // equivalent

// Attribute values
string lang = xpath["book/@lang"];

// Indexed access (1-based in path)
string second = xpath["book[2]/title"];

// Typed access
int port    = xpath.get_value_int("server/@port", 80);
bool debug  = xpath.get_value_bool("server/@debug", false);
double rate = xpath.get_value_real("server/@rate", 1.0);

// Get all matching elements
list<XML::Element *> books = xpath.get_elements("book");

// Get single element pointer (nullptr if not found)
XML::Element *server = xpath.get_element("server");
```

### Modifying via XPath

```cpp
XML::XPathProcessor xpath(root);

// Set values
xpath.set_value("server/@port", "443");
xpath.set_value_int("server/@port", 443);
xpath.set_value_bool("server/@debug", false);

// Add elements
xpath.add_element("server", new XML::Element("listener", "port", "80"));
XML::Element *cache = xpath.add_element("server", "cache");

// Ensure a path exists (creates intermediate elements)
XML::Element *deep = xpath.ensure_path("server/cache/memory");

// Delete elements
xpath.delete_elements("server/old-setting");

// Replace an element
xpath.replace_element("server/cache", new XML::Element("cache", "type", "redis"));
```

### Read-Only XPath

Use `ConstXPathProcessor` when you only need to read from a const Element:

```cpp
void inspect(const XML::Element& root)
{
  XML::ConstXPathProcessor xpath(root);
  string val = xpath["setting/@value"];
  int count = xpath.get_value_int("items/@count", 0);
}
```

### Configuration Files

The `Configuration` class wraps parsing and XPath with file management:

```cpp
// Read from file
XML::Configuration config("/etc/myapp/config.xml");
if (!config.read("config"))  // optional: validate root element name
{
  cerr << "Failed to read config\n";
  return;
}

// Query values using XPath
string host = config["server/@host"];
int port    = config.get_value_int("server/@port", 8080);
bool debug  = config.get_value_bool("debug/@enabled", false);

// Get a list of values
list<string> paths = config.get_values("search/path");

// Get a name->value map
// e.g. <env><var name="HOME">/home/user</var>...</env>
map<string, string> env = config.get_map("env/var", "name");

// Direct root access
XML::Element& root = config.get_root();
```

#### File Fallback

```cpp
// Try multiple locations
list<string> paths = { "/etc/myapp/config.xml",
                       "~/.myapp/config.xml",
                       "config.xml" };
XML::Configuration config(paths);
config.read();  // reads first file that exists
```

#### Include Processing

Given a config file:
```xml
<config>
  <include file="db.xml"/>
  <include file="conf.d/*.xml"/>
  <server host="localhost"/>
</config>
```

```cpp
XML::Configuration config("config.xml");
config.read();
config.process_includes("id");  // merges included files by "id" attribute
```

#### Modify and Write Back

```cpp
config.set_value_int("server/@port", 9090);
config.add_element("server", new XML::Element("cache", "size", "256"));
config.write();  // atomic write back to file
```

#### Overlay Configuration

```cpp
config.read();
config.superimpose_file("overrides.xml", "id");
```

### Superimpose (Deep Merge)

Overlay one element tree onto another, matching children by an identifier attribute:

```cpp
XML::Element base("root");
base.add("item", "id", "1").set_attr("colour", "red");
base.add("item", "id", "2").set_attr("colour", "green");

XML::Element overlay("root");
overlay.add("item", "id", "1").set_attr("colour", "blue");  // replaces
overlay.add("item", "id", "3").set_attr("colour", "yellow"); // added

base.superimpose(overlay, "id");
// item id=1 now has colour=blue
// item id=2 unchanged (colour=green)
// item id=3 added (colour=yellow)
```

### Copying Elements

```cpp
// Shallow copy (name, content, attributes only)
XML::Element *shallow = element.copy();

// Deep copy (includes all descendants)
XML::Element *deep = element.deep_copy();

// Copy to existing element
XML::Element target;
element.deep_copy_to(target);
```

### Tree Manipulation

```cpp
// Detach from parent
child.detach();

// Replace an element in its parent
child.replace_with(new XML::Element("replacement"));

// Remove specific children
root.remove_children("deprecated-tag");

// Clear all children
root.clear_children();

// Translate element names via map
map<string, string> trans;
trans["old-name"] = "new-name";  // rename
trans["remove-me"] = "";         // delete
root.translate(trans);

// Add/remove namespace prefixes recursively
root.add_prefix("ns:");     // "foo" -> "ns:foo"
root.remove_prefix("ns:");  // "ns:foo" -> "foo"
```

### Serialization

```cpp
// To string
string xml = root.to_string();              // without <?xml?> declaration
string xml_pi = root.to_string(true);       // with <?xml version="1.0"?>

// To stream
root.write_to(cout);
cout << root;  // equivalent

// Start/end tags only (for streaming output)
root.write_start_to(cout);  // <root attr="val">
cout << "streaming content";
root.write_end_to(cout);    // </root>
```

### Template Expansion

The `Expander` class generates text from an XML template and a values document:

```cpp
// Template
string templ_xml =
  "<html>"
  "  <h1><expand:replace value='title'/></h1>"
  "  <expand:each element='item'>"
  "    <p><expand:index/>. <expand:replace value='@name'/></p>"
  "  </expand:each>"
  "  <expand:if value='footer'>"
  "    <footer><expand:replace value='footer'/></footer>"
  "  </expand:if>"
  "</html>";

XML::Parser tp;
tp.read_from(templ_xml);

// Values
string values_xml =
  "<data>"
  "  <title>My Page</title>"
  "  <item name='Alpha'/>"
  "  <item name='Beta'/>"
  "  <footer>Copyright 2024</footer>"
  "</data>";

XML::Parser vp;
vp.read_from(values_xml);

// Expand
XML::Expander expander(tp.get_root());
string result = expander.expand(vp.get_root());
```

#### Template Tags Reference

| Tag | Description |
|-----|-------------|
| `<expand:replace value="xpath"/>` | Insert XPath value from values document |
| `<expand:replace var="name"/>` | Insert variable value |
| `<expand:if value="xpath">...</expand:if>` | Conditional (truthy = begins with T/t/Y/y/1) |
| `<expand:unless value="xpath">...</expand:unless>` | Inverse conditional |
| `<expand:ifeq value="xpath" to="str">...</expand:ifeq>` | Equality conditional |
| `<expand:ifne value="xpath" to="str">...</expand:ifne>` | Inequality conditional |
| `<expand:each element="xpath">...</expand:each>` | Loop over matching elements |
| `<expand:index from="1"/>` | Current loop index (default base 1) |
| `<expand:set var="name">...</expand:set>` | Set scoped variable |

### Namespace Handling

```cpp
// Normalise namespace prefixes during parsing
XML::Parser parser;
parser.fix_namespace("http://example.com/ns", "ex");
parser.read_from(xml);
// All elements in http://example.com/ns now use "ex:" prefix
```

### Parser Options

```cpp
// Default: content optimisation enabled
XML::Parser p1;

// Preserve whitespace
XML::Parser p2(XML::PARSER_PRESERVE_WHITESPACE);

// Lenient mode (allow bare & and < in non-XML contexts)
XML::Parser p3(XML::PARSER_BE_LENIENT | XML::PARSER_OPTIMISE_CONTENT);

// Custom error stream
XML::Parser p4(my_error_stream, XML::PARSER_OPTIMISE_CONTENT);
```

## API Reference

### Element Class

#### Constructors

| Signature | Description |
|-----------|-------------|
| `Element()` | Empty element |
| `Element(name)` | Named element |
| `Element(name, content)` | Named with text content |
| `Element(name, attr, value)` | Named with one attribute |
| `Element(name, attr, value, content)` | Named with attribute and content |
| `Element(const Element&)` | Deep copy |

#### Validity

| Method | Description |
|--------|-------------|
| `valid()` | Returns `true` if not `Element::none` |
| `operator bool()` | Same as `valid()` |
| `operator!()` | Same as `!valid()` |

#### Child Access

| Method | Returns | Description |
|--------|---------|-------------|
| `get_child(n)` | `Element&` | n'th child (any type), or `none` |
| `get_child_element(n)` | `Element&` | n'th element child (skips text), or `none` |
| `get_child(name, n)` | `Element&` | n'th named child, or `none` |
| `make_child(name)` | `Element&` | Get or create named child |
| `get_descendant(name)` | `Element&` | First descendant with name, or `none` |
| `get_children()` | `list<const Element*>` | All children |
| `get_children(name)` | `list<Element*>` | All named children |
| `get_descendants(name, prune)` | `list<Element*>` | All named descendants |

#### Attribute Access

| Method | Returns | Description |
|--------|---------|-------------|
| `get_attr(name, def)` | `string` | Attribute value or default |
| `operator[](name)` | `string` | Same as `get_attr(name)` |
| `get_attr_bool(name, def)` | `bool` | Boolean attribute |
| `get_attr_int(name, def)` | `int` | Integer attribute |
| `get_attr_hex(name, def)` | `int` | Hex integer attribute |
| `get_attr_int64(name, def)` | `uint64_t` | 64-bit integer attribute |
| `get_attr_hex64(name, def)` | `uint64_t` | 64-bit hex attribute |
| `get_attr_real(name, def)` | `double` | Floating-point attribute |
| `has_attr(name)` | `bool` | Attribute exists? |
| `get_attrs_with_prefix(prefix)` | `map<string,string>` | Prefix-filtered attributes |

#### Attribute Setters (all return `Element&` for chaining)

| Method | Description |
|--------|-------------|
| `set_attr(name, value)` | Set string attribute |
| `set_attr_int(name, value)` | Set integer attribute |
| `set_attr_hex(name, value)` | Set hex attribute |
| `set_attr_int64(name, value)` | Set 64-bit integer |
| `set_attr_hex64(name, value)` | Set 64-bit hex |
| `set_attr_bool(name, value)` | Set boolean (yes/no) |
| `set_attr_real(name, value)` | Set real number |
| `remove_attr(name)` | Remove attribute |

#### Content

| Method | Returns | Description |
|--------|---------|-------------|
| `get_content()` | `string` | Direct child text |
| `operator*()` | `string` | Same as `get_content()` |
| `get_deep_content()` | `string` | Recursive text from subtree |
| `get_xpath()` | `string` | XPath from root to this element |

#### Output

| Method | Description |
|--------|-------------|
| `write_to(stream, with_pi)` | Write full XML to stream |
| `to_string(with_pi)` | Convert to XML string |
| `write_start_to(stream)` | Write opening tag |
| `start_to_string()` | Opening tag as string |
| `write_end_to(stream)` | Write closing tag |
| `end_to_string()` | Closing tag as string |

#### Tree Operations

| Method | Description |
|--------|-------------|
| `add(...)` | Add child (multiple overloads) |
| `add_xml(xml)` | Parse and add XML text |
| `merge_xml(xml)` | Parse and merge XML text |
| `copy()` / `copy_to(dest)` | Shallow copy |
| `deep_copy()` / `deep_copy_to(dest)` | Deep copy |
| `superimpose(source, id)` | Deep merge by identifier |
| `merge(source)` | Copy attrs and children from source |
| `translate(map)` | Rename/delete elements via map |
| `add_prefix(prefix)` | Add namespace prefix recursively |
| `remove_prefix(prefix)` | Remove namespace prefix recursively |
| `detach()` | Remove from parent |
| `replace_with(element)` | Replace self in parent |
| `remove_children(name)` | Delete named children |
| `clear_children()` | Delete all children |
| `optimise()` | Snap single text child to content |

### Parser Class

| Method | Description |
|--------|-------------|
| `Parser(stream, flags)` | Construct with error stream and flags |
| `Parser(flags)` | Construct with cerr and flags |
| `fix_namespace(name, prefix)` | Map namespace URI to prefix |
| `read_from(istream)` | Parse from stream (throws `ParseFailed`) |
| `read_from(string)` | Parse from string (throws `ParseFailed`) |
| `get_root()` | Get root element (or `Element::none`) |
| `detach_root()` | Detach and return root (caller owns) |
| `replace_root(element)` | Replace root element |

### Stream Operators

```cpp
istream& operator>>(istream& s, Parser& p);     // cin >> parser
ostream& operator<<(ostream& s, const Element& e); // cout << element
```

## Build

The library is built with Tup as part of the ObTools build system:

```
NAME    = ot-xml
TYPE    = lib
DEPENDS = ot-file ot-log
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
