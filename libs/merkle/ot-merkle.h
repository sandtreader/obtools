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

// Hash function template - takes left and right child hash and returns
// the combined hash for a branch node
template <typename T>      // T = Hash type
using HashFunc = T (*)(const T& left_hash, const T& right_hash);

//==========================================================================
// Merkle Tree Node abstract class
template <typename T>  // T = Hash type
class Node
{
public:
  uint64_t index{0};  // In flattened complete tree 2i+1, 2i+2 scheme
  virtual bool is_leaf() const = 0;
  virtual T get_hash() const = 0;

  // Lambda function for tree traversal, taking a Node
  using TraversalCallbackFunc = function<void(const Node<T>&)>;

  virtual void traverse_preorder(
          const typename Node<T>::TraversalCallbackFunc& callback) const = 0;
  virtual void push_children(queue<Node<T>*>&) const {}

  // Set index according to 2i+1, 2i+2 scheme
  virtual void set_index(uint64_t _index) = 0;

  virtual ~Node() {};
};

//==========================================================================
// Merkle Tree Leaf
template <typename T>       // T = Hash type
class Leaf: public Node<T>
{
private:
  T hash;

public:
  Leaf(const T& _hash):
    hash(_hash)
  {}

  bool is_leaf() const override { return true; }

  T get_hash() const override
  {
    return hash;
  }

  void traverse_preorder(
      const typename Node<T>::TraversalCallbackFunc& callback) const override
  {
    callback(*this);
  }

  void set_index(uint64_t _index) override { this->index = _index; }
};

//==========================================================================
// Merkle Tree Branch
template <typename T>       // T = Hash type
class Branch: public Node<T>
{
private:
  HashFunc<T> hash_func;
  unique_ptr<Node<T>> left;
  unique_ptr<Node<T>> right;

public:
  Branch(
    HashFunc<T> _hash_func,
    unique_ptr<Node<T>>& _left,
    unique_ptr<Node<T>>& _right
  ):
    hash_func{_hash_func}, left{std::move(_left)}, right{std::move(_right)}
  {}
  Branch(
    HashFunc<T> _hash_func,
    unique_ptr<Node<T>>& _left
  ):
    hash_func{_hash_func}, left{std::move(_left)}
  {}
  Branch(Branch&& b):
    hash_func{b.hash_func}, left{std::move(b.left)}, right{std::move(b.right)}
  {}

  bool is_leaf() const override { return false; }

  T get_hash() const override
  {
    if (right)
      return hash_func(left->get_hash(), right->get_hash());
    else
      return left->get_hash();
  }

  void traverse_preorder(
      const typename Node<T>::TraversalCallbackFunc& callback) const override
  {
    callback(*this);
    if (left) left->traverse_preorder(callback);
    if (right) right->traverse_preorder(callback);
  }

  void push_children(queue<Node<T>*>& queue) const override
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
template<typename T>       // T = Hash type
class Tree
{
private:
  unique_ptr<Node<T>> root;

  // Get the parent layer for a layer of nodes, connecting them up
  // in the tree of Branches
  static vector<unique_ptr<Node<T>>> get_parent_nodes(
    const HashFunc<T> hash_func,
    vector<unique_ptr<Node<T>>>& nodes)
  {
    vector<unique_ptr<Node<T>>> parents;
    for (auto i = 0u; i < nodes.size(); i+=2)
    {
      if (i+1 < nodes.size())  // Both left and right
        parents.push_back(make_unique<Branch<T>>(hash_func, nodes[i],
                                                 nodes[i + 1]));
      else                     // Only left
        parents.push_back(make_unique<Branch<T>>(hash_func, nodes[i]));
    }
    return parents;
  }

  // Build a tree from a vector of leaf hashes - returns the root node
  static unique_ptr<Node<T>> build_tree(
    const HashFunc<T> hash_func,
    const vector<T>& leaves)
  {
    vector<unique_ptr<Node<T>>> nodes;
    for (auto i = 0u; i < leaves.size(); ++i)
      nodes.push_back(make_unique<Leaf<T>>(leaves[i]));
    while (nodes.size() > 1)
      nodes = get_parent_nodes(hash_func, nodes);
    return std::move(nodes[0]);
  }

public:
  Tree(const HashFunc<T> hash_func, const vector<T>& leaves):
    root{build_tree(hash_func, leaves)}
  {
    root->set_index(0);
  }
  Tree(Tree&& t):
    root(std::move(t.root))
  {}

  T get_hash() const
  {
    return root->get_hash();
  }

  // Walk the tree from the root - preorder
  void traverse_preorder(
      const typename Node<T>::TraversalCallbackFunc& callback) const
  {
    root->traverse_preorder(callback);
  }

  // Walk the tree from the root - breadth first
  virtual void traverse_breadth_first(
      const typename Node<T>::TraversalCallbackFunc& callback) const
  {
    queue<Node<T>*> queue;
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

//==========================================================================
// Common hash types
namespace Hash
{
  // SHA256
  struct SHA256
  {
    using hash_t = vector<byte>;
    static hash_t hash_func(const hash_t& left_hash,
                            const hash_t& right_hash);
  };
}

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_MERKLE_H
