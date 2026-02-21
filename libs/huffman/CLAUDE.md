# CLAUDE.md - ObTools::Huffman Library

## Overview

`ObTools::Huffman` provides Huffman coding and decoding with support for multi-tree contexts and bit-level reading. Lives under `namespace ObTools::Huffman`.

**Header:** `ot-huffman.h`
**Dependencies:** `ot-chan`, `ot-text`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Value` | Huffman value (byte or special: start/stop/escape) |
| `Node` | Tree node (leaf with value or fork with children) |
| `Tree` | Single Huffman tree for encoding/decoding |
| `MultiTree` | Context-dependent multi-tree decoder |
| `MultiReader` | Parses mapping file format into MultiMappings |
| `Mapping` | Bit sequence to value mapping |
| `MultiMapping` | Context-aware mapping (extends Mapping with index) |

## Value

```cpp
enum class Value::Special { none, start, stop, escape };
Value();                         // invalid
Value(unsigned char _value);     // byte value
Value(Special _svalue);          // special value
bool is_special() const;
unsigned char get_value() const;
Special get_special_value() const;
```

## Node

```cpp
Node();                                     // fork node
Node(const Value& _value);                  // leaf node
bool is_leaf() const;
void set_node(bool bit, const Node& node);  // set 0/1 child
const Node *get_node(bool bit) const;
Node& ensure_node(const vector<bool>& sequence);
```

## Tree

```cpp
bool add_mapping(const Mapping& m);
bool read_value(Channel::BitReader& reader, Value& value) const;
bool read_value(const vector<bool>& sequence, Value& value) const;
```

## MultiTree

```cpp
bool populate_from(MultiReader& reader);
bool read_value(const Value& index, Channel::BitReader& reader, Value& value) const;
bool read_string(Channel::BitReader& reader, string& s) const;
```

## MultiReader File Format

```
previous_value:bit_sequence:value:
START:00:T:          # From START, "00" = 'T'
p:101:ESCAPE:        # From 'p', "101" = ESCAPE
x:01:0x3a:           # Hex value
```
