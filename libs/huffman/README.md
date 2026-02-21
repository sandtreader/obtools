# ObTools::Huffman

Huffman coding and decoding with support for multi-tree contexts and bit-level reading.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-huffman.h"
using namespace ObTools;

// Build a simple Huffman tree
Huffman::Tree tree;
Huffman::Mapping m;
m.value = Huffman::Value('A');
m.sequence = {false};  // bit 0 = 'A'
tree.add_mapping(m);

m.value = Huffman::Value('B');
m.sequence = {true};   // bit 1 = 'B'
tree.add_mapping(m);

// Decode from bit sequence
Huffman::Value v;
vector<bool> bits = {false};
tree.read_value(bits, v);  // v = 'A'
```

### Context-Dependent Decoding

```cpp
// Load multi-tree mappings from file
// Format: previous_value:bit_sequence:value:
// START:00:H:
// H:01:i:
// i:1:STOP:

ifstream fs("mappings.txt");
Huffman::MultiReader reader(fs);
Huffman::MultiTree mtree;
mtree.populate_from(reader);

// Decode a string from a bit reader
Channel::BitReader bit_reader(data, len);
string result;
mtree.read_string(bit_reader, result);
```

## Build

```
NAME    = ot-huffman
TYPE    = lib
DEPENDS = ot-chan ot-text
```

## License

Copyright (c) 2017 Paul Clark. MIT License.
