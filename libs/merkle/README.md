# ObTools::Merkle

Generic Merkle tree implementation with support for sum trees, custom hash functions, and tree traversals.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-merkle.h"
using namespace ObTools;

// Create a SHA256 Merkle tree
using HashT = Merkle::Hash::SHA256::hash_t;
using MyTree = Merkle::Tree<HashT, Merkle::Hash::SHA256::hash_func>;

vector<HashT> leaves = { hash1, hash2, hash3, hash4 };
MyTree tree(leaves);

// Get root hash
auto root_hash = tree.get_hash();

// Traverse the tree
tree.traverse_preorder([](const auto& node) {
  if (node.is_leaf())
    cout << "Leaf at index " << node.index << endl;
});

tree.traverse_breadth_first([](const auto& node) {
  cout << "Node at index " << node.index << endl;
});
```

### Merkle Sum Tree

```cpp
// Sum tree aggregates data along the tree
// Useful for proof-of-reserves and similar applications
using MySumTree = Merkle::SumTree<
  HashT, LeafData, AggregateData,
  leaf_data_agg_func, leaf_hash_func,
  branch_hash_func, branch_data_agg_func>;

vector<LeafData> leaves = { ... };
MySumTree tree(leaves);
auto root_data = tree.get_data();  // aggregated from all leaves
```

## Build

```
NAME    = ot-merkle
TYPE    = lib
DEPENDS = ot-crypto
```

## License

Copyright (c) 2022 Paul Clark. MIT License.
