# CLAUDE.md - ObTools::XML Library

## Overview

`ObTools::XML` is a custom XML parser/DOM/XPath library providing a non-standard but practical DOM, a streaming parser, simplified XPath for configuration files, and a template-based text expander. It lives under `namespace ObTools::XML`.

**Header:** `ot-xml.h`
**Dependencies:** `ot-file`, `ot-log`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Element` | Core DOM node - an XML document is a tree of Elements |
| `Parser` | Streaming XML parser that builds an Element tree |
| `XPathProcessor` | Read-write simplified XPath queries over an Element tree |
| `ConstXPathProcessor` | Read-only XPath queries |
| `Configuration` | High-level config file reader with XPath access and file I/O |
| `Expander` | Template-based text expansion using XPath values |
| `ElementIterator` | Forward iterator over `list<Element *>` |
| `ConstElementIterator` | Const-correct forward iterator |

## Parser Flags

Combinable with bitwise OR:

| Flag | Value | Effect |
|------|-------|--------|
| `PARSER_OPTIMISE_CONTENT` | 1 | Snap single text children to `parent.content` (default) |
| `PARSER_PRESERVE_WHITESPACE` | 2 | Keep all whitespace as-is |
| `PARSER_FIX_NAMESPACES` | 4 | Enable namespace prefix translation |
| `PARSER_BE_LENIENT` | 8 | Allow bare `&` and `<` in non-XML contexts |

## Element Public Members

```
string name;                 // Tag name (empty for text nodes)
string content;              // Text content (optimised or data nodes)
map<string,string> attrs;    // Attributes
list<Element *> children;    // Child elements
Element *parent;             // Parent pointer
int line;                    // Source line number
static Element none;         // Sentinel for "not found"
```

## Element API Summary

### Constructors
- `Element()` - empty
- `Element(name)` - named
- `Element(name, content)` - named with content
- `Element(name, attr_name, attr_value)` - named with one attribute
- `Element(name, attr_name, attr_value, content)` - all four
- Copy constructor and `operator=` do deep copies

### Validity
- `element.valid()` / `if (element)` / `if (!element)` - check against `Element::none`

### Adding Children
- `add(Element*)` - take ownership of pointer
- `add(const Element&)` - deep copy and add
- `add(name)`, `add(name, content)`, `add(name, attr, val)`, `add(name, attr, val, content)`
- `add_xml(xml_string)` - parse XML text and add as child
- `merge_xml(xml_string)` - parse and merge into this element

### Finding Children
- `get_child(n)` - n'th child (any type)
- `get_child_element(n)` - n'th element child (skips text)
- `get_child(name, n)` - n'th child with given name
- `make_child(name)` - get or create named child
- `get_descendant(name)` - first descendant with name (recursive)
- `get_children()` - all children as list
- `get_children(name)` - all named children
- `get_descendants(name, prune)` - all named descendants (optionally pruned)

### Attributes
- `get_attr(name, default)` / `element["attr"]` - string
- `get_attr_bool(name, default)` - bool (recognises T/t/Y/y/1)
- `get_attr_int(name, default)` - int
- `get_attr_hex(name, default)` - hex int
- `get_attr_int64(name, default)` / `get_attr_hex64(name, default)` - 64-bit
- `get_attr_real(name, default)` - double
- `has_attr(name)` - existence check
- `get_attrs_with_prefix(prefix)` - map of prefix-stripped attrs
- `set_attr(name, value)` - all `set_attr_*` variants return `*this` for chaining
- `remove_attr(name)`

### Content
- `get_content()` / `*element` - direct child text
- `get_deep_content()` - recursive text from entire subtree
- `get_xpath()` - path from root to this element

### Serialization
- `write_to(stream, with_pi)` / `to_string(with_pi)` - full element
- `write_start_to(stream)` / `start_to_string()` - opening tag only
- `write_end_to(stream)` / `end_to_string()` - closing tag only
- `cout << element` - stream operator

### Tree Manipulation
- `copy_to(dest)` / `copy()` - shallow copy
- `deep_copy_to(dest)` / `deep_copy()` - recursive copy
- `superimpose(source, identifier)` - overlay merge with child matching
- `merge(source)` - copy attrs and children from source
- `translate(map)` - rename/delete elements via map
- `add_prefix(prefix)` / `remove_prefix(prefix)` - namespace prefix ops
- `detach()` - remove from parent
- `replace_with(element)` - replace self in parent
- `remove_children(name)` / `clear_children()` - delete children
- `optimise()` - snap single text children to content

## XPath Syntax

Simplified XPath supporting child and attribute axes only:

```
foo              - child element "foo"
foo/bar          - grandchild "bar" under "foo"
foo[2]           - second "foo" element (1-indexed)
foo/bar/@attr    - attribute "attr" of "bar" under "foo"
/foo/bar         - absolute path (leading / ignored, always rooted at processor root)
```

### XPathProcessor Methods

**Read (also on ConstXPathProcessor):**
- `get_elements(path)` - list of matching elements
- `get_element(path)` - first match (or nullptr)
- `get_value(path, default)` / `xpath["path"]` - string value
- `get_value_bool`, `get_value_int`, `get_value_hex`, `get_value_int64`, `get_value_hex64`, `get_value_real`

**Write (XPathProcessor only):**
- `set_value(path, value)` - set content or attribute
- `set_value_bool`, `set_value_int`, `set_value_hex`, `set_value_int64`, `set_value_hex64`, `set_value_real`
- `delete_elements(path)` - remove elements
- `add_element(path, Element*)` / `add_element(path, name)` - add child at path
- `ensure_path(path)` - create path if it doesn't exist
- `replace_element(path, Element*)` - replace element at path

## Configuration Class

Wraps `Parser` + `XPathProcessor` with file I/O.

- Constructors accept filename(s) and error stream
- `read(expected_root_name)` / `read_text(xml, expected_root_name)` - parse
- `reload()` - re-read from file
- `process_includes(id_attr)` - process `<include file="..."/>` tags
- `superimpose_file(filename, id_attr)` - overlay another config file
- `write()` - atomic write-back to file
- All `get_value_*`, `set_value_*`, `add_element`, `delete_elements`, `ensure_path`, `replace_element` methods delegate to XPathProcessor
- `get_values(path)` - list of all matching content strings
- `get_map(path, name_attr)` - string->string map from elements
- `replace_root(name)` - replace root element
- `move_file(filename)` - change underlying file path

## Expander Template Language

Tags in the `expand:` namespace:

| Tag | Attributes | Description |
|-----|------------|-------------|
| `<expand:replace>` | `value` or `var` | Insert XPath value or variable |
| `<expand:if>` | `value` or `var` | Expand children if truthy |
| `<expand:unless>` | `value` or `var` | Expand children if falsy |
| `<expand:ifeq>` | `value`/`var`, `to` | Expand if equal to `to` |
| `<expand:ifne>` | `value`/`var`, `to` | Expand if not equal to `to` |
| `<expand:each>` | `element` | Loop over XPath matches |
| `<expand:index>` | `from` (optional) | Current loop index |
| `<expand:set>` | `var` | Set scoped variable to expanded content |

## File Layout

```
ot-xml.h           - Public header (all classes)
element.cc         - Element class implementation
parser.cc          - XML Parser implementation
xpath.cc           - XPath processor implementation
config.cc          - Configuration class implementation
expand.cc          - Expander implementation
test-xml.cc        - Element and parser tests (gtest)
test-xpath.cc      - XPath tests (gtest)
test-config.cc     - Configuration tests (gtest)
tests/             - Test data XML files
Tupfile            - Build configuration
```

## Common Patterns

```cpp
// Parse XML from string
XML::Parser parser;
parser.read_from("<root><item id='1'>Hello</item></root>");
XML::Element& root = parser.get_root();

// Navigate with get_child
XML::Element& item = root.get_child("item");
if (item.valid())
  cout << item["id"] << ": " << *item << endl;  // "1: Hello"

// Iterate children
for (XML::Element::iterator it(root.children); it; ++it)
  cout << it->name << endl;

// XPath queries
XML::XPathProcessor xpath(root);
string val = xpath["item/@id"];          // "1"
string content = xpath["item"];          // "Hello"

// Build elements programmatically
XML::Element root("config");
root.add("server", "host", "localhost", "My Server")
    .set_attr_int("port", 8080);

// Configuration file
XML::Configuration config("/etc/app/config.xml");
if (config.read("config"))
{
  int port = config.get_value_int("server/@port", 80);
  string host = config["server/@host"];
}
```
