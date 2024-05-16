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
  virtual T get_hash() const = 0;
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

  T get_hash() const
  {
    return hash;
  }
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
  Branch(Branch&& b):
    hash_func{b.hash_func}, left{std::move(b.left)}, right{std::move(b.right)}
  {}

  T get_hash() const
  {
    if (right)
      return hash_func(left->get_hash(), right->get_hash());
    else
      return left->get_hash();
  }
};

//==========================================================================
// Merkle Tree
template<typename T>       // T = Hash type
class Tree: public Node<T> {
private:
  unique_ptr<Node<T>> root;

  // Get the parent layer for a layer of nodes, connecting them up
  // in the tree of Branches
  static vector<unique_ptr<Node<T>>> get_parent_nodes(
    const HashFunc<T> hash_func,
    vector<unique_ptr<Node<T>>>& nodes)
  {
    vector<unique_ptr<Node<T>>> parents;
    for (auto i = 0u; i < nodes.size() / 2; ++i)
      parents.push_back(make_unique<Branch<T>>(hash_func, nodes[2 * i],
                                               nodes[2 * i + 1]));
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
  {}
  Tree(Tree&& t):
    root(std::move(t.root))
  {}

  T get_hash() const
  {
    return root->get_hash();
  }
};

//==========================================================================
// Common hash types
namespace Hash
{
  // SHA256
  struct SHA256
  {
    using hash_t = vector<uint8_t>;
    static hash_t hash_func(const hash_t& left_hash,
                            const hash_t& right_hash);
  };
}

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_MERKLE_H
