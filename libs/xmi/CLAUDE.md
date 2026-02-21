# CLAUDE.md - ObTools::XMI Library

## Overview

`ObTools::XMI` parses XMI (XML Metadata Interchange) files for UML model deserialisation. Lives under `namespace ObTools::XMI`.

**Header:** `ot-xmi.h`
**Dependencies:** `ot-xml`, `ot-uml`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Reader` | XMI file parser with UML/XML element ID mapping |
| `ParseFailed` | Parse failure exception |

## Reader

```cpp
Reader(ostream& serr);
void read_from(istream& in);
void record_uml_element(const string& id, UML::Element *e);
UML::Element *lookup_uml_element(const string& id);
XML::Element *lookup_xml_element(const string& id);

// Public members
UML::Model *model;           // parsed UML model
int xmi_version;              // detected XMI version
map<string, UML::Classifier *> class_map;
```
