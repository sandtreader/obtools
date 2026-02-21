# CLAUDE.md - ObTools::Merkle Library

## Overview

`ObTools::Merkle` provides a generic Merkle tree implementation with support for sum trees and custom hash functions. Lives under `namespace ObTools::Merkle`.

**Header:** `ot-merkle.h`
**Dependencies:** `ot-crypto`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Node<HashT, DataT>` | Abstract tree node (leaf or branch) |
| `Leaf<...>` | Leaf node with value and hash |
| `Branch<...>` | Branch node combining two children |
| `GenericTree<...>` | Full Merkle tree from vector of leaves |
| `Tree<...>` | Standard Merkle tree (alias) |
| `SumTree<...>` | Merkle Sum Tree with data aggregation |

## Template Function Types

```cpp
LeafHashFuncT<HashT, LeafDataT>         // hash leaf data
BranchHashFuncT<NodeT>                   // hash two children
BranchSingleHashFuncT<NodeT>             // hash single child
LeafDataAggregationFuncT<DataT, LeafDataT>
BranchDataAggregationFuncT<NodeT>
```

## Node (abstract)

```cpp
virtual bool is_leaf() const = 0;
virtual HashT get_hash() const = 0;
virtual DataT get_data() const = 0;
virtual void traverse_preorder(const TraversalCallbackFunc& callback) const = 0;
```

## GenericTree

```cpp
GenericTree(const vector<LeafDataT>& leaves);
auto get_hash() const;
auto get_data() const;
void traverse_preorder(const TraversalCallbackFunc& callback) const;
void traverse_breadth_first(const TraversalCallbackFunc& callback) const;
```

## Built-in Hash

```cpp
namespace Hash {
  struct SHA256 {
    using hash_t = vector<byte>;
    static hash_t hash_func(const Node<hash_t, void>& left,
                            const Node<hash_t, void>& right);
  };
}
```
