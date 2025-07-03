//==========================================================================
// ObTools::Merkle: ot-merkle.h
//
// Public definitions for ObTools::Merkle
//
// Merkle tree tools
//
// Copyright (c) 2024 Paul Clark.
//==========================================================================

#ifndef __OBTOOLS_MERKLE_H
#define __OBTOOLS_MERKLE_H

#include <memory>
#include <vector>
#include <queue>
#include <functional>

namespace ObTools { namespace Merkle {

// Make our lives easier without polluting anyone else
using namespace std;

// Hash function template - takes left and right children and returns
// the combined hash for a branch node
template <typename HashT, typename NodeT>
using HashFunc = HashT (*)(const NodeT& left, const NodeT& right);

//==========================================================================
// Merkle Tree Node abstract class
template <typename HashT, typename DataT = void>
class Node
{
public:
  uint64_t index{0};  // In flattened complete tree 2i+1, 2i+2 scheme
  virtual bool is_leaf() const = 0;
  virtual HashT get_hash() const = 0;
  virtual DataT get_data() const = 0;

  // Lambda function for tree traversal, taking a Node
  using TraversalCallbackFunc = function<void(const Node<HashT, DataT>&)>;

  virtual void traverse_preorder(
      const Node<HashT, DataT>::TraversalCallbackFunc& callback) const = 0;
  virtual void push_children(queue<Node<HashT, DataT>*>&) const {}

  // Set index according to 2i+1, 2i+2 scheme
  virtual void set_index(uint64_t _index) = 0;

  virtual ~Node() {};
};

//==========================================================================
// Simple leaf value is hash func
template <typename HashT>
HashT leaf_data_is_hash_func(const HashT& hash)
{
  return hash;
}

//==========================================================================
// Null data function
template <typename LeafDataT>
void null_leaf_data_aggregation_func(const LeafDataT&) {}

//==========================================================================
// Merkle Tree Leaf
template <typename HashT, typename LeafDataT = HashT>
using LeafHashFuncT = HashT (*)(const LeafDataT& value);

template<typename DataT, typename LeafDataT>
using LeafDataAggregationFuncT = DataT (*)(const LeafDataT& leaf_data);

template <typename HashT, typename DataT = void, typename LeafDataT = HashT,
          LeafDataAggregationFuncT<DataT, LeafDataT> LeafDataAggregationFunc=
              null_leaf_data_aggregation_func,
          LeafHashFuncT<HashT, LeafDataT> LeafHashFunc =
              leaf_data_is_hash_func<HashT>>
class Leaf: public Node<HashT, DataT>
{
private:
  LeafDataT leaf_data;

public:
  Leaf(const LeafDataT& _leaf_data):
    leaf_data(_leaf_data)
  {}

  bool is_leaf() const override { return true; }

  HashT get_hash() const override
  {
    return LeafHashFunc(leaf_data);
  }

  DataT get_data() const override
  {
    return LeafDataAggregationFunc(leaf_data);
  }

  void traverse_preorder(
    const typename Node<HashT, DataT>::TraversalCallbackFunc& callback
    ) const override
  {
    callback(*this);
  }

  void set_index(uint64_t _index) override { this->index = _index; }
};

//==========================================================================
// Null data function
template <class NodeT>
void null_branch_data_aggregation_func(const NodeT&, const NodeT&) {}

//==========================================================================
// Default single child hash function is just the hash of the left child
template <typename HashT, typename NodeT>
HashT pass_through_single_child_hash(const NodeT& left)
{
  return left.get_hash();
}

//==========================================================================
// Merkle Tree Branch
template <typename NodeT>
using BranchHashFuncT = decltype(((NodeT *)nullptr)->get_hash())
                        (*)(const NodeT& left, const NodeT& right);

template <typename NodeT>
using BranchSingleHashFuncT = decltype(((NodeT *)nullptr)->get_hash())
                              (*)(const NodeT& left);

template <typename NodeT>
using BranchDataAggregationFuncT = decltype(((NodeT *)nullptr)->get_data())
                                   (*)(const NodeT& left, const NodeT& right);

template <typename HashT, typename DataT,
          BranchHashFuncT<Node<HashT, DataT>> BranchHashFunc,
          BranchDataAggregationFuncT<Node<HashT, DataT>>
            BranchDataAggregationFunc = nullptr,
          BranchSingleHashFuncT<Node<HashT, DataT>> BranchSingleHashFunc
            = pass_through_single_child_hash>
class Branch: public Node<HashT, DataT>
{
private:
  unique_ptr<Node<HashT, DataT>> left;
  unique_ptr<Node<HashT, DataT>> right;

public:
  Branch(
    unique_ptr<Node<HashT, DataT>>& _left,
    unique_ptr<Node<HashT, DataT>>& _right
  ):
    left{std::move(_left)}, right{std::move(_right)}
  {}
  Branch(unique_ptr<Node<HashT, DataT>>& _left):
    left{std::move(_left)}
  {}
  Branch(Branch&& b):
    left{std::move(b.left)}, right{std::move(b.right)}
  {}

  bool is_leaf() const override { return false; }

  HashT get_hash() const override
  {
    if (right)
      return BranchHashFunc(*left, *right);
    else
      return BranchSingleHashFunc(*left);
  }

  DataT get_data() const override
  {
    if (right)
      return BranchDataAggregationFunc(*left, *right);
    else
      return left->get_data();
  }

  void traverse_preorder(
    const typename Node<HashT,
                        DataT>::TraversalCallbackFunc& callback) const override
  {
    callback(*this);
    if (left) left->traverse_preorder(callback);
    if (right) right->traverse_preorder(callback);
  }

  void push_children(queue<Node<HashT, DataT>*>& queue) const override
  {
    if (left) queue.push(left.get());
    if (right) queue.push(right.get());
  }

  void set_index(uint64_t _index) override
  {
    this->index = _index;
    if (left) left->set_index(2*_index + 1);
    if (right) right->set_index(2*_index + 2);
  }
};

//==========================================================================
// Merkle Tree
template<typename LeafDataT, class NodeT, class LeafT, class BranchT>
class GenericTree
{
protected:
  unique_ptr<NodeT> root;

  // Get the parent layer for a layer of nodes, connecting them up
  // in the tree of Branches
  static vector<unique_ptr<NodeT>> get_parent_nodes(
    vector<unique_ptr<NodeT>>& nodes)
  {
    vector<unique_ptr<NodeT>> parents;
    for (auto i = 0u; i < nodes.size(); i+=2)
    {
      if (i+1 < nodes.size())  // Both left and right
        parents.push_back(make_unique<BranchT>(nodes[i], nodes[i + 1]));
      else                     // Only left
        parents.push_back(make_unique<BranchT>(nodes[i]));
    }
    return parents;
  }

  // Build a tree from a vector of leaf hashes - returns the root node
  static unique_ptr<NodeT> build_tree(const vector<LeafDataT>& leaves)
  {
    vector<unique_ptr<NodeT>> nodes;
    for (auto i = 0u; i < leaves.size(); ++i)
      nodes.push_back(make_unique<LeafT>(leaves[i]));
    while (nodes.size() > 1)
      nodes = get_parent_nodes(nodes);
    return std::move(nodes[0]);
  }

public:
  GenericTree(const vector<LeafDataT>& leaves):
    root{build_tree(leaves)}
  {
    root->set_index(0);
  }
  GenericTree(GenericTree&& t):
    root(std::move(t.root))
  {}

  auto get_hash() const
  {
    return root->get_hash();
  }

  auto get_data() const
  {
    return root->get_data();
  }

  // Walk the tree from the root - preorder
  void traverse_preorder(
      const typename NodeT::TraversalCallbackFunc& callback) const
  {
    root->traverse_preorder(callback);
  }

  // Walk the tree from the root - breadth first
  virtual void traverse_breadth_first(
      const typename NodeT::TraversalCallbackFunc& callback) const
  {
    queue<NodeT *> queue;
    queue.push(root.get());

    while (!queue.empty())
    {
      auto node = queue.front();
      queue.pop();

      callback(*node);
      node->push_children(queue);
    }
  }
};

template<typename HashT,
         BranchHashFuncT<Node<HashT, void>> BranchHashFunc,
         LeafHashFuncT<HashT> LeafHashFunc = leaf_data_is_hash_func<HashT>>
using Tree = GenericTree<HashT, Node<HashT, void>, Leaf<HashT>,
                         Branch<HashT, void, BranchHashFunc>>;

//==========================================================================
// Merkle Sum Tree

template<typename HashT, typename LeafDataT, typename DataT,
         LeafDataAggregationFuncT<DataT, LeafDataT> LeafDataAggregationFunc,
         LeafHashFuncT<HashT, LeafDataT> LeafHashFunc,
         BranchHashFuncT<Node<HashT, DataT>> BranchHashFunc,
         BranchDataAggregationFuncT<Node<HashT, DataT>>
           BranchDataAggregationFunc,
         BranchSingleHashFuncT<Node<HashT, DataT>> BranchSingleHashFunc
           = pass_through_single_child_hash>
using SumTree = GenericTree<LeafDataT, Node<HashT, DataT>,
                            Leaf<HashT, DataT, LeafDataT,
                                 LeafDataAggregationFunc, LeafHashFunc>,
                            Branch<HashT, DataT, BranchHashFunc,
                                   BranchDataAggregationFunc, BranchSingleHashFunc>>;

//==========================================================================
// Common hash types
namespace Hash
{
  // SHA256
  struct SHA256
  {
    using hash_t = vector<byte>;
    static hash_t hash_func(const Node<hash_t, void>& left_hash,
                            const Node<hash_t, void>& right_hash);
  };
}

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_MERKLE_H
